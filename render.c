#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "colorspace.h"

double lerp(double a, double b, double t) {
	return a + (b-a)*t;
}

void rot(int n, int* x, int* y, int rx, int ry) {
	if (ry == 0) {
		if (rx == 1) {
			*x = n-1 - *x;
			*y = n-1 - *y;
		}
		
		int t = *x;
		*x = *y;
		*y = t;
	}
}

void d2xy(int n, int d, int* x, int* y) {
	int rx, ry,s, t=d;
	*x = 0;
	*y = 0;
	for (s=1; s < n; s*=2) {
		rx = 1 & (t/2);
		ry = 1 & (t ^ rx);
		rot(s, x, y, rx, ry);
		*x += s * rx;
		*y += s * ry;
		t /= 4;
	}
}

int main() {
	for (int i = 0; i < 15; i++) {
		int x, y;
		d2xy(4, i, &x, &y);
	}
	
	int width  = 1000;
	int height = 1000;
	
	float colormul = 6;
	
	int frames = 1000;
	int s_frame = 0;
	int e_frame = frames;
	
	int is_in_set;
	
	char fn[32];
	
	char* buf = (char*) malloc(width*3*height + 32);
	int n;
	
	printf("malloc'd %d bytes @ 0x%016lx\n", width*3*height + 32, (unsigned long) buf);
	
	FILE* fout;
	
	float start = (float) clock() / CLOCKS_PER_SEC;
	
	for (int i = s_frame; i < e_frame;  i++) {
		n = sprintf(buf, "P6\n%d %d\n255\n", width, height);
		
		int X, Y;
		d2xy(36, (int) ((float) i/(frames-1) * 1295), &X, &Y);
		double cr_d = (double) X/35 * 2 - 1;
		double ci_d = (double) Y/35 * 2 - 1;
		printf("%d -> %d -> (%d, %d) -> %.2lf+%.2lfi\n", i, (int) ((float) i/(frames-1) * 1295), X, Y, cr_d, ci_d);
		
		double zr, zi, cr, ci, zr_t;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				cr = cr_d;
				ci = ci_d;
			
				zr = (double) x / width  * 4 - 2;
				zi = (double) y / height * 4 - 2;
			
				is_in_set = 1;
				int i;
				for (i = 0; i < 1000; i++) {
					zr_t = zr*zr - zi*zi + cr;
					zi   = zr*zi*2 + ci;
					
					zr = zr_t;
				
					if (zr*zr+zi*zi > 4) {
						is_in_set = 0;
						break;
					}
				}
				
				if (is_in_set) {
					buf[n] = 0; n++;
					buf[n] = 0; n++;
					buf[n] = 0; n++;
				}
				else {
					float modulus = sqrt(zr*zr + zi*zi);
					float mu = i + 2 - (log(log(modulus))) / log(2.0);
					hsv col_hsv = {fmod(mu * colormul, 360), 1, 1};
					rgb col_rgb = hsv2rgb(col_hsv);
					buf[n] = (int) (col_rgb.r * 255); n++;
					buf[n] = (int) (col_rgb.g * 255); n++;
					buf[n] = (int) (col_rgb.b * 255); n++;
				}
			}
		}
		
		sprintf(fn, "frames/%03d.pgm", i);
		FILE* fout = fopen(fn, "wb");
		fwrite(buf, 1, n, fout);
		fclose(fout);
	}
	
	float end = (float) clock() / CLOCKS_PER_SEC;
	
	float tme = end-start;
	int m = floor(tme/60);
	float s = fmod(tme, 60);
	printf("Complete in %dm %.2fs\n", m, s);
	
	free(buf);
	
	return 0;
}
