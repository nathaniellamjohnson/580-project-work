import os
env = Environment(ENV = os.environ)

env.Append(CXXFLAGS=["-std=c++11","-g","-Wall","-O3"])
env.Append(LINKFLAGS=["-L/usr/local/lib"])

env.Program("ray_tracer",
            [
                "camera.cpp","hierarchy.cpp",
                "flat_shader.cpp","main.cpp","parse.cpp",
                "phong_shader.cpp","plane.cpp","reflective_shader.cpp",
                "render_world.cpp","sphere.cpp","box.cpp","mesh.cpp"
            ])

