
#include "tiny_obj_loader.h"
#include "stb_image_write.h"
#include "stb_image.h"

#define PAR

#ifdef PAR
#include <omp.h>
#endif



#ifdef GUI
#include "gui.h"
#else
#include "tui.h"
#endif
