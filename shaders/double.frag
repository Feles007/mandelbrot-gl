#version 460 core
#extension GL_ARB_gpu_shader_fp64: require

uniform uint window_width;
uniform uint window_height;

uniform uint iterations;

uniform dvec4 extents;

out vec4 color;

uint mandelbrot(double cr, double ci, uint iterations) {
	uint i = uint(0);
	double x = 0, y = 0;
	while ((x*x + y*y <= 4) && (i < iterations)) {
		double new_x = x*x - y*y + cr;
		y = 2 * x * y + ci;
		x = new_x;
		++i;
	}
	return i;
}
uint mandelbrot_unroll(double cr, double ci, uint iterations) {
	const uint unroll = uint(32);
	uint i = uint(0);
	double x = 0, y = 0, x2 = 0, y2 = 0;
	while ((x2 + y2 <= 4) && (i < iterations)) {
		for (uint j = 0; j < unroll; ++j) {
			y = 2 * x * y + ci;
			x = x2 - y2 + cr;
			x2 = x * x;
			y2 = y * y;
			++i;
		}
	}
	return i;
}
uint mandelbrot_test(double cr, double ci, uint iterations) {
	uint i = uint(0);
	uint cache_age = uint(0);
	double cache_x = 0, cache_y = 0;
	double x = 0, y = 0;
	while ((x*x + y*y <= 4) && (i < iterations)) {
		double new_x = x*x - y*y + cr;
		y = 2 * x * y + ci;
		x = new_x;
		++i;

		if (abs(cache_x - x) < 0.001 && abs(cache_y - y) < 0.001) {
			return 0;
		}

		++cache_age;
		if (cache_age > 20) {
			cache_age = 0;
			cache_x = x;
			cache_y = y;
		}
	}
	return i;
}
void main() {
	double scaled_y = mix(extents.x, extents.y, double(gl_FragCoord.y) / double(window_height));
	double scaled_x = mix(extents.z, extents.w, double(gl_FragCoord.x) / double(window_width));
	uint i = mandelbrot(scaled_x, scaled_y, iterations);
	
	
	float h = mix(0, 359, float(i) / float(iterations));

	float hp = h / 60;
	float z = 1 - abs(mod(hp, 2) - 1);

	float r, g, b;

	if      (hp < 1) { r = 1; g = z, b = 0; }
	else if (hp < 2) { r = z; g = 1, b = 0; }
	else if (hp < 3) { r = 0; g = 1, b = z; }
	else if (hp < 4) { r = 0; g = z, b = 1; }
	else if (hp < 5) { r = z; g = 0, b = 1; }
	else if (hp < 6) { r = 0; g = 0, b = z; } // r 1->0
	
	color = vec4(r, g, b, 1);
}
