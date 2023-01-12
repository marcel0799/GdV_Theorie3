//=============================================================================

#include "Subdivision_Viewer.h"

//=============================================================================

int main(int argc, char **argv)
{
    Subdivision_Viewer viewer("SubdivisionViewer", 800, 600);
    if (argc > 1)
        viewer.load_mesh(argv[1]);
    return viewer.run();
}
