#include <stdio.h>
#include <curses.h>
#include <math.h>


/// modified content from: 
//  tire de :  https://gist.github.com/JohnnyonFlame

int rows, cols; 

float pos[2] = {(6 * 32) + 16.f, (6 * 32) + 16.f};
int dir = 0;
int is3d = 1;

	int yaw, i, j, grid;
	float fov ; //= 60.f / 180.f * M_PI;
	float fov_step ; //= fov / 80.f;
	float s, c, t, dx, dy, d;
	float proj[4];
	char ch = 0;

char dungeon[17][17] =
{
"aaaaaaaaaaaaaaaa",
"a a            a",
"a a aaaa a a   a",
"a a a          a",
"a aaaaa        a",
"a a            a",
"a aaaaaaaa  a  a",
"a           aa a",
"a           a  a",
"a  aaaaaa  aaaaa",
"a     a        a",
"a     a        a",
"a     a        a",
"a     a        a",
"a              a",
"aaaaaaaaaaaaaaaa",
};

char dungeon_visited[17][17] = {[0 ... 16] = {[0 ... 16] = 0}};

char dir_ch[] = {'>','v','<','^'};
char wall_ch[] = {249, 176, 177, '$', '@', '#', 'T', 'w', 'a', '%', '*', '+', ',', '.', ' '};




 float f_dist(float *s0, float *s1)
{
	float dx = s1[0] - s0[0], dy = s1[1] - s0[1];

	return (dx*dx + dy*dy);
}


void grid_snap_x(float *v, float s, float c, float t)
{
	float snap[2];

	snap[0]  = (floorf(v[0] / 32.f) * 32.f);
	snap[0]	+= ((c > 0) ? 32.01f : -.01f);
	snap[1]  = fabs((snap[0] - v[0]) * t) * ((s>0)?1:-1);
	snap[1] += v[1];

	v[0] = snap[0];
	v[1] = snap[1];
}

void grid_snap_y(float *v, float s, float c, float t)
{
	float snap[2];

	snap[1]  = (floorf(v[1] / 32.f) * 32.f);
	snap[1]	+= ((s > 0) ? 32.01f : -.01f);
	snap[0]  = fabs((snap[1] - v[1]) / t) * ((c>0)?1:-1);
	snap[0] += v[0];

	v[0] = snap[0];
	v[1] = snap[1];
}

void grid_snap(float *v, float s, float c, float t)
{
	grid_snap_x(&v[0], s, c, t);
	grid_snap_y(&v[2], s, c, t);
}

int grid_step_x(float *v, float s, float c, float t)
{
	v[0] = v[0] +  ((c>0) ? 32.f : -32.f);
	v[1] = v[1] + (((s>0) ? 32.f : -32.f) * t);
}

int grid_step_y(float *v, float s, float c, float t)
{
	v[1] = v[1] +  ((s>0) ? 32.f : -32.f);
	v[0] = v[0] + (((c>0) ? 32.f : -32.f) / t);
}

int pos_move(float *v, float s, int d)
{
	float npos[2] = {v[0], v[1]};

	switch (d)
	{
	case 0: npos[0]+=s; break;
	case 1: npos[1]+=s; break;
	case 2: npos[0]-=s; break;
	case 3: npos[1]-=s; break;
	default: break;
	}

	if (dungeon[(int)floorf(npos[1] / 32.f)][(int)floorf(npos[0] / 32.f)] == ' ')
	{
		v[0] = npos[0];
		v[1] = npos[1];
	}
}

void maze_draw()
{
                erase();


		if (!is3d)
		{
			for (i=0; i<17; i++)
			{
				for (j=0; j<17; j++)
				{
					if (dungeon_visited[i][j])
						mvaddch(i, j, dungeon[i][j]);
				}
			}
		}

		for (yaw=-40; yaw<40;yaw++)
		{
			proj[0] = pos[0];
			proj[1] = pos[1];
			proj[2] = pos[0];
			proj[3] = pos[1];

			s = sinf((fov_step*yaw) + ((M_PI)*(dir/2.f)));
			c = cosf((fov_step*yaw) + ((M_PI)*(dir/2.f)));
			t = fabs(s/c);

			grid_snap(proj, s, c, t);

			for (i=0; i<16; i++)
			{
				float dx, dy;
				dx = f_dist(pos, &proj[0]);
				dy = f_dist(pos, &proj[2]);

				if (dx < dy)
				{
					d = sqrtf(dx) * cosf(fov_step*yaw);
					grid = 0;
				}
				else
				{
					d = sqrtf(dy) * cosf(fov_step*yaw);
					grid = 2;
				}

				int _x = floorf((proj[grid])   / 32.f),
					  _y = floorf((proj[grid+1]) / 32.f);

				if (((_x>=0)&&(_x<16)) && ((_y>=0)&&(_y<16)))
				{
					if (dungeon[_y][_x] != ' ')
					{
						if (is3d)
						{
							if (grid)
								attron(A_BOLD);
							else
								attroff(A_BOLD);

							float sz = 32.f / d * 32.f;
							for (j=-floorf(sz/2.f);j<ceilf(sz/2.f);j++)
							{
								if (((j+12>=0) && (j+12<35)))
								{
									int dd = floorf(d/16.f);
									mvaddch(j+12, yaw+40, wall_ch[(dd < sizeof(wall_ch))
											? dd
											: sizeof(wall_ch) - 1]);
								}
							}

							attroff(A_BOLD);
						}
						else
						{
							attron(A_BOLD);
							mvaddch(_y, _x, dungeon[_y][_x]);
							attroff(A_BOLD);
						}

						dungeon_visited[_y][_x] = 1;
						break;
					}
					else if (!is3d)
						mvaddch(_y, _x, '.');
				}

				if (dx < dy)
					grid_step_x(&proj[0], s, c, t);
				else
					grid_step_y(&proj[2], s, c, t);
			}
		}

		if (!is3d)
			mvaddch(floorf(pos[1]/32.f), floorf(pos[0]/32.f), dir_ch[dir]);

	} 

int main(int argc, char *argv[])
{
	initscr();
	curs_set(0);

	fov = 60.f / 180.f * M_PI;
	fov_step  = fov / 80.f;


        int gameover = 0; 
        pos[0] = 400; pos[1] = 144;  dir = 2; 
        while ( gameover == 0 ) 
        {
                getmaxyx( stdscr, rows, cols);
                maze_draw(); 
                mvprintw( 0, 0, "[%d][%f][%f][%f]", dir, pos[0], pos[1],pos[2] ) ;
                ch = getch(); 
		switch(tolower(ch))
		{
			case 'q':	gameover = 1; break;
			case '3':	is3d = !is3d; break;

			case 'd':	dir = (dir+1)%4; break;
			case 'a':	dir = (dir+3)%4; break;
			case 'w':	pos_move(pos, 32.f, dir);	break;
			case 's':	pos_move(pos, -32.f, dir); break;

			case 'k':	pos_move(pos,  32.f, dir);	break;
			case 'j':	pos_move(pos, -32.f, dir); break;

			case 'l':	dir = (dir+1)%4; break;
			case 'h':	dir = (dir+3)%4; break;

			default: break;
		}


         }
}



