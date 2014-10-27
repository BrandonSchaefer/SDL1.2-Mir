#include "SDL_mirbuffer.h"

struct QueueNode
{
    TAILQ_ENTRY(QueueNode) entries;
    SDL_Rect* rects;
    int num;
};

void PutPixels(_THIS, int numrects, const SDL_Rect* const rects, const MirGraphicsRegion* region)
{
    char* s_dest = NULL;
    char* pixels = NULL;

    int i, j, x, y, w, h, start;
    int bytes_per_pixel, bytes_per_row, s_stride, d_stride;

    s_stride = SDL_VideoSurface->pitch;
    d_stride = region->stride;

    bytes_per_pixel = SDL_VideoSurface->format->BytesPerPixel;

    for (i = 0; i < numrects; ++i)
    {
        s_dest = region->vaddr;
        pixels = (char*)SDL_VideoSurface->pixels;

        x = rects[i].x;
        y = rects[i].y;
        w = rects[i].w;
        h = rects[i].h;

        if (w <= 0 || h <= 0 || (x + w) <= 0 || (y + h) <= 0)
            continue;

        if (x < 0)
        {
            x += w;
            w += rects[i].x;
        }
        if (y < 0)
        {
            y += h;
            h += rects[i].y;
        }

        if (x + w > SDL_VideoSurface->w)
            w = SDL_VideoSurface->w - x;
        if (y + h > SDL_VideoSurface->h)
            h = SDL_VideoSurface->h - y;

        start = y * s_stride + (x * bytes_per_pixel);
        pixels += start;
        s_dest += start;

        bytes_per_row =  bytes_per_pixel * w;
        for (j = 0; j < h; j++)
        {
            memcpy(s_dest, pixels, bytes_per_row);
            pixels += s_stride;
            s_dest += d_stride;
        }
    }
}

void RedrawRegion(_THIS, const MirGraphicsRegion* region)
{
    int h;

    int d_stride = region->stride;
    int s_stride = SDL_VideoSurface->pitch;

    char* dest = region->vaddr;
    char* src  = (char*)SDL_VideoSurface->pixels;

    int bytes_per_pixel = SDL_VideoSurface->format->BytesPerPixel;
    int bytes_per_row =  bytes_per_pixel * SDL_VideoSurface->w;
    for (h = 0; h < SDL_VideoSurface->h; h++)
    {
        memcpy(dest, src, bytes_per_row);
        dest += d_stride;
        src += s_stride;
    }
}

SDL_Rect* DeepCopyRects(int numrects, const SDL_Rect* const rects)
{
    int n;
    SDL_Rect* new_rects = (SDL_Rect*)SDL_calloc(1, (numrects) * sizeof(SDL_Rect));

    for (n = 0; n < numrects; n++)
    {
        new_rects[n].x = rects[n].x;
        new_rects[n].y = rects[n].y;
        new_rects[n].w = rects[n].w;
        new_rects[n].h = rects[n].h;
    }

    return new_rects;
}

// If one of our frames in the queue is an exact match, just use that pointer
SDL_Rect* FindDuplicateRects(const struct Queue* const queue, int numrects, const SDL_Rect* const rects)
{
    int n;
    int i;
    int match;

    struct QueueNode* node = NULL;

    for (n = 0; n < numrects; n++)
    {
        for (node = queue->head.tqh_first; node != NULL; node = node->entries.tqe_next)
        {
            match = 1;
            if (node->num == numrects)
            {
                for (i = 0; i < numrects; i++)
                {
                    if (node->rects[i].x != rects[i].x ||
                        node->rects[i].y != rects[i].y ||
                        node->rects[i].w != rects[i].w ||
                        node->rects[i].h != rects[i].h)
                    {
                        match = 0;
                        break;
                    }
                }

                if (match)
                {
                    return node->rects;
                }
            }
        }
    }

    return NULL;
}

// Only deep copy when we can't find an exact copy in the prev frames.
struct QueueNode* NewQueueNode(const struct Queue* const queue, int numrects, const SDL_Rect* const rects)
{
    struct QueueNode* new_node = SDL_calloc(1, sizeof(struct QueueNode));

    new_node->rects = FindDuplicateRects(queue, numrects, rects);
    if (new_node->rects == NULL)
        new_node->rects = DeepCopyRects(numrects, rects);

    new_node->num = numrects;

    return new_node;
}

void InsertNewQueueNode(struct Queue* const queue, int numrects, const SDL_Rect* const rects)
{
    struct QueueNode* new_node = NewQueueNode(queue, numrects, rects);
    TAILQ_INSERT_TAIL(&queue->head, new_node, entries);
}

SDL_bool IsRectsPointerStillInQueue(const struct Queue* const queue, const SDL_Rect* const rect_ptr)
{
    struct QueueNode* node;

    for (node = queue->head.tqh_first; node != NULL; node = node->entries.tqe_next)
    {
        if (node->rects == rect_ptr)
        {
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}

// Only delete our rects when such a prev frame is not currenlty in use.
void DeleteQueueNode(struct Queue* queue, struct QueueNode* queue_node)
{
    TAILQ_REMOVE(&queue->head, queue_node, entries);

    if (IsRectsPointerStillInQueue(queue, queue_node->rects) == SDL_FALSE)
        SDL_free(queue_node->rects);

    SDL_free(queue_node);
}

void Mir_UpdateRects(_THIS, int numrects, SDL_Rect* rects)
{
    if (!mir_surface_is_valid(this->hidden->surface))
    {
          const char* error = mir_surface_get_error_message(this->hidden->surface);
          fprintf(stderr, "Failed to created a mir surface: %s", error);
          return;
    }

    MirGraphicsRegion region;
    mir_surface_get_graphics_region(this->hidden->surface, &region);

    MirNativeBuffer* buffer;
    mir_surface_get_current_buffer(this->hidden->surface, &buffer);

    struct Queue* queue = this->hidden->buffer_queue;
    struct QueueNode* node;
    int age = buffer->age;

    if (age > 0)
    {
        PutPixels(this, numrects, rects, &region);
        node = queue->head.tqh_first;

        while (age >= 0 && node != NULL)
        {
            PutPixels(this, node->num, node->rects, &region);

            node = node->entries.tqe_next;
            age--;
        }

        DeleteQueueNode(queue, queue->head.tqh_first);

        InsertNewQueueNode(queue, numrects, rects);
    }
    else
    {
        RedrawRegion(this, &region);

        InsertNewQueueNode(queue, numrects, rects);
    }

    mir_surface_swap_buffers_sync(this->hidden->surface);
}

void Mir_InitQueue(struct Queue* const queue)
{
    TAILQ_INIT(&queue->head);
}

void Mir_DeleteQueue(struct Queue* const queue)
{
    struct QueueNode* node, *node_next;
    node = queue->head.tqh_first;

    while (node != NULL)
    {
        node_next = node->entries.tqe_next;
        DeleteQueueNode(queue, node);
        node = node_next;
    }
}
