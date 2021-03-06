#include <VGA2.h>

VGA2 tft(A2, A0);

int zOff = 150;
int xOff = 0;
int yOff = 0;
int cSize = 50;
int view_plane = 160;
float angleX = 0;
float angleY = 0;
float angleZ = 0;
int hres = 240;
int vres = 240;
int hpos = 160;
int vpos = 120;
unsigned char frame = 0;
unsigned long timer = 0;

float cube3d[8][3] = {
	{(float)(xOff - cSize), (float)(yOff + cSize), (float)(zOff - cSize)},
	{(float)(xOff + cSize), (float)(yOff + cSize), (float)(zOff - cSize)},
	{(float)(xOff - cSize), (float)(yOff - cSize), (float)(zOff - cSize)},
	{(float)(xOff + cSize), (float)(yOff - cSize), (float)(zOff - cSize)},
	{(float)(xOff - cSize), (float)(yOff + cSize), (float)(zOff + cSize)},
	{(float)(xOff + cSize), (float)(yOff + cSize), (float)(zOff + cSize)},
	{(float)(xOff - cSize), (float)(yOff - cSize), (float)(zOff + cSize)},
	{(float)(xOff + cSize), (float)(yOff - cSize), (float)(zOff + cSize)}
};
unsigned int cube2d[8][2];

unsigned int oldCube2d[8][2];

void setup() {
	
	tft.initializeDevice();
	timer = millis()+1000;
	frame = 0;
	tft.fillScreen(Color::Black);
}

void loop() {
	xrotate(angleX);
	yrotate(angleY);
	zrotate(angleZ);
	printcube();
    tft.flip();
	angleX = 0.01;//double(analogRead(0))/1024-0.5;
	angleY = 0.02;//double(analogRead(1))/1024-0.5;
	angleZ = 0.005;//double(analogRead(2))/1024-0.5;
}

void printcube() {
	for(byte i = 0; i < 8; i++) {
		cube2d[i][0] = (unsigned int)((cube3d[i][0] * view_plane / cube3d[i][2]) + (hpos));
		cube2d[i][1] = (unsigned int)((cube3d[i][1] * view_plane / cube3d[i][2]) + (vpos));
	}

    tft.fillScreen(Color::Black);
    draw_cube(cube2d);

	for (byte i = 0; i < 8; i++) {
		oldCube2d[i][0] = cube2d[i][0];
		oldCube2d[i][1] = cube2d[i][1];
	}
}

void zrotate(float q) {
	float tx,ty,temp;
	for(byte i = 0; i < 8; i++) {
		tx = cube3d[i][0] - xOff;
		ty = cube3d[i][1] - yOff;
		temp = tx * cos(q) - ty * sin(q);
		ty = tx * sin(q) + ty * cos(q);
		tx = temp;
		cube3d[i][0] = tx + xOff;
		cube3d[i][1] = ty + yOff;
	}
}

void yrotate(float q) {
	float tx,tz,temp;
	for(byte i = 0; i < 8; i++) {
		tx = cube3d[i][0] - xOff;
		tz = cube3d[i][2] - zOff;
		temp = tz * cos(q) - tx * sin(q);
		tx = tz * sin(q) + tx * cos(q);
		tz = temp;
		cube3d[i][0] = tx + xOff;
		cube3d[i][2] = tz + zOff;
	}
}

void xrotate(float q) {
	float ty,tz,temp;
	for(byte i = 0; i < 8; i++) {
		ty = cube3d[i][1] - yOff;
		tz = cube3d[i][2] - zOff;
		temp = ty * cos(q) - tz * sin(q);
		tz = ty * sin(q) + tz * cos(q);
		ty = temp;
		cube3d[i][1] = ty + yOff;
		cube3d[i][2] = tz + zOff;
	}
}

void draw_cube(unsigned int c2d[8][2]) {
	tft.drawLine(c2d[0][0],c2d[0][1],c2d[1][0],c2d[1][1], Color::White);
	tft.drawLine(c2d[0][0],c2d[0][1],c2d[2][0],c2d[2][1], Color::White);
	tft.drawLine(c2d[0][0],c2d[0][1],c2d[4][0],c2d[4][1], Color::White);
	tft.drawLine(c2d[1][0],c2d[1][1],c2d[5][0],c2d[5][1], Color::White);
	tft.drawLine(c2d[1][0],c2d[1][1],c2d[3][0],c2d[3][1], Color::White);
	tft.drawLine(c2d[2][0],c2d[2][1],c2d[6][0],c2d[6][1], Color::White);
	tft.drawLine(c2d[2][0],c2d[2][1],c2d[3][0],c2d[3][1], Color::White);
	tft.drawLine(c2d[4][0],c2d[4][1],c2d[6][0],c2d[6][1], Color::White);
	tft.drawLine(c2d[4][0],c2d[4][1],c2d[5][0],c2d[5][1], Color::White);
	tft.drawLine(c2d[7][0],c2d[7][1],c2d[6][0],c2d[6][1], Color::White);
	tft.drawLine(c2d[7][0],c2d[7][1],c2d[3][0],c2d[3][1], Color::White);
	tft.drawLine(c2d[7][0],c2d[7][1],c2d[5][0],c2d[5][1], Color::White);

	tft.drawLine(c2d[0][0],c2d[0][1],c2d[5][0],c2d[5][1], Color::Red);
	tft.drawLine(c2d[1][0],c2d[1][1],c2d[4][0],c2d[4][1], Color::Green);
	tft.drawLine(c2d[2][0],c2d[2][1],c2d[7][0],c2d[7][1], Color::Blue);
	tft.drawLine(c2d[3][0],c2d[3][1],c2d[6][0],c2d[6][1], Color::Yellow);
}

void erase_cube(unsigned int c2d[8][2]) {
    tft.drawLine(c2d[0][0],c2d[0][1],c2d[1][0],c2d[1][1], Color::Black);
    tft.drawLine(c2d[0][0],c2d[0][1],c2d[2][0],c2d[2][1], Color::Black);
    tft.drawLine(c2d[0][0],c2d[0][1],c2d[4][0],c2d[4][1], Color::Black);
    tft.drawLine(c2d[1][0],c2d[1][1],c2d[5][0],c2d[5][1], Color::Black);
    tft.drawLine(c2d[1][0],c2d[1][1],c2d[3][0],c2d[3][1], Color::Black);
    tft.drawLine(c2d[2][0],c2d[2][1],c2d[6][0],c2d[6][1], Color::Black);
    tft.drawLine(c2d[2][0],c2d[2][1],c2d[3][0],c2d[3][1], Color::Black);
    tft.drawLine(c2d[4][0],c2d[4][1],c2d[6][0],c2d[6][1], Color::Black);
    tft.drawLine(c2d[4][0],c2d[4][1],c2d[5][0],c2d[5][1], Color::Black);
    tft.drawLine(c2d[7][0],c2d[7][1],c2d[6][0],c2d[6][1], Color::Black);
    tft.drawLine(c2d[7][0],c2d[7][1],c2d[3][0],c2d[3][1], Color::Black);
    tft.drawLine(c2d[7][0],c2d[7][1],c2d[5][0],c2d[5][1], Color::Black);

    tft.drawLine(c2d[0][0],c2d[0][1],c2d[5][0],c2d[5][1], Color::Black);
    tft.drawLine(c2d[1][0],c2d[1][1],c2d[4][0],c2d[4][1], Color::Black);
    tft.drawLine(c2d[2][0],c2d[2][1],c2d[7][0],c2d[7][1], Color::Black);
    tft.drawLine(c2d[3][0],c2d[3][1],c2d[6][0],c2d[6][1], Color::Black);
}
