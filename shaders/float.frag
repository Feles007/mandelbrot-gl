#version 460 core

uniform uint window_width;
uniform uint window_height;

uniform uint iterations;

uniform vec4 extents;

out vec4 color;

uint mandelbrot(float cr, float ci, uint iterations) {
	uint i = uint(0);
	float x = 0, y = 0;
	while ((x*x + y*y <= 4) && (i < iterations)) {
		float new_x = x*x - y*y + cr;
		y = 2 * x * y + ci;
		x = new_x;
		++i;
	}
	return i;
}
void main() {
	float scaled_y = mix(extents.x, extents.y, float(gl_FragCoord.y) / float(window_height));
	float scaled_x = mix(extents.z, extents.w, float(gl_FragCoord.x) / float(window_width));
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
