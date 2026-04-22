#include <memory>
#include <string>

#include <emscripten/emscripten.h>

#include "render_world.h"

void Parse(Render_World& world,int& width,int& height,const char* test_file);

bool debug_pixel = false;
bool disable_hierarchy = false;

static std::unique_ptr<Render_World> g_world;
static int g_width = 0;
static int g_height = 0;

extern "C"
{
EMSCRIPTEN_KEEPALIVE
void render_scene_file(const char* scene_path)
{
    g_world.reset(new Render_World());
    g_width = 0;
    g_height = 0;

    Parse(*g_world, g_width, g_height, scene_path);
    g_world->Render();
}

EMSCRIPTEN_KEEPALIVE
unsigned int* get_pixels()
{
    return g_world ? g_world->camera.colors : nullptr;
}

EMSCRIPTEN_KEEPALIVE
int get_width()
{
    return g_width;
}

EMSCRIPTEN_KEEPALIVE
int get_height()
{
    return g_height;
}
}