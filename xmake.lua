local glad_dir = "D:/Programming/Libraries/glad-4.6-core-GL_ARB_gpu_shader_fp64/"
local glfw_dir = "D:/Programming/Libraries/glfw-3.4/"
local fstd_dir = "D:/Programming/Libraries/fstd/"

add_rules("mode.debug", "mode.release")

target("mandelbrot-gl")
	set_kind("binary")
	add_files("src/*.cpp")

	set_rundir(".")

	add_syslinks("user32")
	add_syslinks("Gdi32")
	add_syslinks("Shell32")

	add_includedirs(glfw_dir .. "include")
	add_linkdirs(glfw_dir .. "build/windows/x64/release")
	add_links("glfw3")

	add_includedirs(glad_dir .. "include")
	add_files(glad_dir .. "src/glad.c")

	add_includedirs(fstd_dir .. "include")
