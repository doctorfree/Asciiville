#include "bores/bores.h"
#include "vlayer.h"

/* callback that returns a cell from a given layer */
static void _get_cell_LYR(Cell *r_cell, int x, int y, void *ud) {
   Layer *l = (Layer*) ud;
   *r_cell = l->cells[x][y];
}

/* callback that returns a cell from a document composite */
static void _get_cell_COMPOSITE(Cell *r_cell, int x, int y, void *ud) {
   Document *doc = (Document*) doc;
   *r_cell = document_calc_effective_cell(doc, x, y);
}

VirtualLayer *vlayer_create(int width, int height, vlayer_get_cell_t get_cell,
                                                        void *user_data){
   VirtualLayer *vl = zalloc(sizeof(VirtualLayer));
   vl->width = width;
   vl->height = height;
   vl->get_cell = get_cell;
   vl->user_data = user_data;
   return vl;
}

VirtualLayer *vlayer_create_from_layer(Layer *l) {
   return vlayer_create(l->width, l->height, _get_cell_LYR, (void*) l);
}

VirtualLayer *vlayer_create_from_composite(Document *doc) {
   if (doc->layer_count <= 0) return NULL;
   return vlayer_create(doc->layers[0]->width, doc->layers[0]->height,
                        _get_cell_COMPOSITE, (void*) doc);
}

void vlayer_destroy(VirtualLayer *vl) {
   if (!vl) return;
   sfree(vl);
}

void vlayer_get_cell(VirtualLayer *vl, Cell *r_cell, int x, int y) {
   if (x < 0 || x >= vl->width || y < 0 || y >= vl->height) {
      r_cell->ch = ' ';
      r_cell->attr = 0x70;
   }
   else vl->get_cell(r_cell, x, y, vl->user_data);
}

