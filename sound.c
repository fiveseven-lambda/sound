#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

unsigned int samplerate = 44100;
#define channel 2
#define bitdepth 16

struct Wave{
	int type;
	double frequency, velocity[2];
} sound[100];

double length = 3;

void read_wave(FILE *fp){
	char ch;
	int i = 0;
	double j;
	int flag = 0;
	double *hoge;
	while(1){
		switch(ch = getc(fp)){
		case EOF:
			return;
		case ';':
			++i;
			break;
		case '<':
			flag |= 1;
			flag &= ~2;
			break;
		case '>':
			flag &= ~1;
			flag &= ~2;
			flag &= ~4;
			break;
		case '.':
			flag |= 2;
			j = 1;
			break;
		case ',':
			flag |= 4;
			flag &= ~2;
			j = 1;
			break;
		case '0' ... '9':
			hoge = (flag & 1 ? &(sound[i].frequency) : &(sound[i].velocity[(flag >> 2) & 1]));
			if(flag & 2) *hoge += (ch - '0') * (j /= 10);
			else *hoge = *hoge * 10 + (ch - '0');
		}
	}
}

void write16bit(short *dest);

int main(int argc, char *argv[]){
	char *out_filename = NULL;
	if(!out_filename) out_filename = "a.wav";

	FILE *fp = fopen(argv[1], "r");
	read_wave(fp);
	
	//for(int i = 0; i < 10; ++i) printf("%f %f %f\n", sound[i].frequency, sound[i].velocity[0], sound[i].velocity[1]);

	int out = open(out_filename, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
	int mapsize = length * samplerate * bitdepth / 8 * channel + 44;
	lseek(out, mapsize - 1, SEEK_SET);
	char c = 0;
	write(out, &c, 1);
	lseek(out, 0, SEEK_SET);
	char *map = mmap(NULL, mapsize, PROT_WRITE, MAP_SHARED, out, 0);
	((char *)map)[0] = 'R';
	((char *)map)[1] = 'I';
	((char *)map)[2] = 'F';
	((char *)map)[3] = 'F';
	((unsigned int *)map)[1] = mapsize - 8;
	((char *)map)[8] = 'W';
	((char *)map)[9] = 'A';
	((char *)map)[10] = 'V';
	((char *)map)[11] = 'E';
	((char *)map)[12] = 'f';
	((char *)map)[13] = 'm';
	((char *)map)[14] = 't';
	((char *)map)[15] = ' ';
	((unsigned int *)map)[4] = 16;
	((unsigned short *)map)[10] = 1;
	((unsigned short *)map)[11] = channel;
	((unsigned int *)map)[6] = samplerate;
	((unsigned int *)map)[7] = samplerate * bitdepth / 8 * channel;
	((unsigned short *)map)[16] = channel * bitdepth / 8;
	((unsigned short *)map)[17] = bitdepth;
	((char *)map)[36] = 'd';
	((char *)map)[37] = 'a';
	((char *)map)[38] = 't';
	((char *)map)[39] = 'a';
	((unsigned int *)map)[10] = mapsize - 44;
	memset(map + 44, 0, mapsize - 44);
	write16bit((short *)(map + 44));
	close(out);
	munmap(map, mapsize);
}

void write16bit(short *dest){
	for(int i = 0; i < 100; ++i){
		for(int j = 0; j < samplerate * length; ++j){
			for(int k = 0; k < channel; ++k){
				dest[j * channel + k] += sin(2 * M_PI * sound[i].frequency * 440 * j / samplerate) * (
						sound[i].velocity[0]
						* (1 - (double)j / samplerate / length)
						+ sound[i].velocity[1]
						* ((double)j / samplerate / length)
				)
						* (1 << 15) - 1;
			}
		}
	}
}
