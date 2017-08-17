#ifdef _M_IX86
#include <windows.h>
#else
#include <stream.h>
#endif

#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <math.h>
#include <time.h>
#include <sstream>
#include <string>

/*---------------------------------------------------------------------------------------*/
/*  Asteroids																		     */
/*																						 */	
/*- Modification History: ---------------------------------------------------------------*/
/*  When:	Who:			Comments:													 */
/*  2-Sep-2015	Kevin Dyer	Initial implementation										 */
/*  24-Sep-2015 Kevin Dyer  Lighting, Player movement/shooting and asteroid movement	 */
/*  23-Oct-2015 Kevin Dyer  Collisions, Explosions, Asteroid splitting                   */
/*---------------------------------------------------------------------------------------*/

/************* NOTE **************/
/* I am currently using a
dodecahedron as a placeholder
for an asteroid, as i could not
get my asteroid to render properly
when converting from wireframe to
polygon (It has a bunch of holes
in it). The code is still there,
it has just been commented out. */
/*********************************/

//  Module global prototypes
void display_things(void);
void display_asteroid(void);
void display_player_ship(void);
void display_bullets(void);  
void display_alien_ship(void);
void timer(int);
void specialKeyboard(unsigned char, int, int);
void specialKeyboardUp(unsigned char, int, int);
void keyboard(unsigned char, int, int);
void keyboardUp(unsigned char, int, int);
void lighting();
void handle_menu(int);
void handle_mouse(int, int, int, int);
void initialize_alien();

//  Module global variables
typedef enum {
  TRANSLATE,
  ROTATE_X,
  ROTATE_Y,
  ROTATE_Z
} mode;

int    btn[ 3 ] = { 0 };		// Current button state
mode   cur_mode = TRANSLATE;		// Current mouse mode
int    mouse_x, mouse_y;		// Current mouse position
float  rotate[ 16 ] = {			// Current composite rotation matrix
         1,0,0,0,			//  OpenGL matrices are column major
         0,1,0,0,
         0,0,1,0,
         0,0,0,1
       };
float  translate[ 2 ] = { 0, 0 };	// Translation in X and Y

struct game_object {
	float x, y, dx, dy, angle, last_angle, rotation, collision_radius, speed;
	bool initialized, xAxis, yAxis, zAxis;
};

//typedef struct particleData    particleData;

struct explosion {
	game_object particles[50];
	int fuel;
};

const float MAX_SPEED = 0.05;
const float BULLET_SPEED = 0.075;
const float ALIEN_SPEED = 0.01;
const float ASTEROID_SPEED = 3;
const float M_ASTEROID_SPEED = 4;
const float S_ASTEROID_SPEED = 5;
const int NUM_PARTICLES = 50;
float DRAG = 1;
float ACCELERATION = 0;
bool player_start = false;
bool player_moving = false;
bool player_turning_right = false;
bool player_turning_left = false;
const int num_asteroids = 6;
const int num_med_asteroids = num_asteroids * 2;
const int num_small_asteroids = num_med_asteroids * 2;
int cur_max_asteroids = 2;
int med_counter = 0;
int small_counter = 0;
int cur_asteroids = 0;
int respawn_timer = 0;
int alien_timer = 0;
int shoot_timer = 0;
int turn_timer = 0;
int explosion_counter = 0;
long score = 0;
int lives = 3;
long one_up = 10000;

game_object player;
game_object asteroid[num_asteroids];
game_object m_asteroid[num_med_asteroids];
game_object s_asteroid[num_small_asteroids];
game_object alien;
game_object bullet[4];
game_object alien_bullet[1];
explosion explosions[100];

void display_asteroid(int i) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations

	glPushMatrix();			// Save glLookAt transformation

	//Set size and color for asteroids
	glScalef(0.2, 0.2, 0.2);
	glColor3f(0.3, 0.3, 0.3);

	if (asteroid[i].initialized) {
		glTranslatef(asteroid[i].x, asteroid[i].y, 0);
		
		if (asteroid[i].xAxis) {
			glRotatef(asteroid[i].rotation, 1, 0, 0);
		}
		else if (asteroid[i].yAxis) {
			glRotatef(asteroid[i].rotation, 0, 1, 0);
		}
		else if (asteroid[i].zAxis) {
			glRotatef(asteroid[i].rotation, 0, 0, 1);
		}

		//Draw placeholder shape for asteroid
		glutSolidDodecahedron();
		//glutWireSphere((double) asteroid[i].collision_radius, 4, 4);
	}

	glPopMatrix();			// Restore glLookAt
	
}

void display_med_asteroid(int i) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations

	glPushMatrix();			// Save glLookAt transformation

	//Set size and color for asteroids
	glScalef(0.15, 0.15, 0.15);
	glColor3f(0.3, 0.3, 0.3);

	if (m_asteroid[i].initialized) {
		glTranslatef(m_asteroid[i].x, m_asteroid[i].y, 0);

		if (m_asteroid[i].xAxis) {
			glRotatef(m_asteroid[i].rotation, 1, 0, 0);
		}
		else if (m_asteroid[i].yAxis) {
			glRotatef(m_asteroid[i].rotation, 0, 1, 0);
		}
		else if (m_asteroid[i].zAxis) {
			glRotatef(m_asteroid[i].rotation, 0, 0, 1);
		}

		//Draw placeholder shape for asteroid
		glutSolidDodecahedron();
		//glutWireSphere((double)asteroid[i].collision_radius, 4, 4);
	}

	glPopMatrix();			// Restore glLookAt

}

void display_small_asteroid(int i) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations

	glPushMatrix();			// Save glLookAt transformation

	//Set size and color for asteroids
	glScalef(0.1, 0.1, 0.1);
	glColor3f(0.3, 0.3, 0.3);

	if (s_asteroid[i].initialized) {
		glTranslatef(s_asteroid[i].x, s_asteroid[i].y, 0);

		if (s_asteroid[i].xAxis) {
			glRotatef(s_asteroid[i].rotation, 1, 0, 0);
		}
		else if (s_asteroid[i].yAxis) {
			glRotatef(s_asteroid[i].rotation, 0, 1, 0);
		}
		else if (s_asteroid[i].zAxis) {
			glRotatef(s_asteroid[i].rotation, 0, 0, 1);
		}

		//Draw placeholder shape for asteroid
		glutSolidDodecahedron();
	}

	glPopMatrix();			// Restore glLookAt

}

void display_player_ship(void) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glTranslatef(player.x, player.y, 0);
	glRotatef(player.angle, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	glScalef(0.35, 0.35, 0.35);

	//draw purple player ship
	glColor3f(0.5, 0, 1);
	glutSolidCone(0.25, 0.75, 10, 10);

	glPopMatrix();			// Restore glLookAt

}

void display_bullets(void) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	for (int i = 0; i < 4; i++) {
		if (bullet[i].initialized) {
			glVertex3f(bullet[i].x, bullet[i].y, 0);
		}
	}
	glEnd();

	glPopMatrix();			// Restore glLookAt

}

void display_alien_ship(void) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	//move right to draw alien ship
	glTranslatef(alien.x, alien.y, 0);
	glRotatef(90, 1, 0, 0);
	glRotatef(alien.rotation, 0, 0, 1);
	glScalef(0.5, 0.5, 0.5);

	//Draw red alien ship
	glColor3f(0.85, 0.5, 0.1);
	glutSolidTorus(0.15, 0.3, 3, 7);
	glutSolidSphere(0.25, 10, 10);

	glPopMatrix();			// Restore glLookAt

}

void display_alien_bullets(void) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(0, 0, 1);
	for (int i = 0; i < 1; i++) {
		if (alien_bullet[i].initialized) {
			glVertex3f(alien_bullet[i].x, alien_bullet[i].y, 0);
		}
	}
	glEnd();

	glPopMatrix();			// Restore glLookAt

}

void display_explosion(int k) {

	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glPointSize(4.0);
	glBegin(GL_POINTS);
	glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i < NUM_PARTICLES; i++) {
		glVertex3f(explosions[k].particles[i].x, explosions[k].particles[i].y, 0);
	}
	glEnd();

	glPopMatrix();

}

void display_lives() {
	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glTranslatef(-2.8, 3.3, 0);
	glRotatef(90, 0, 1, 0);
	glScalef(0.2, 0.2, 0.2);

	for (int i = 0; i < lives; i++) {
		glTranslatef(0, 0, 1.2);

		//draw purple player ship
		glColor3f(0.5, 0, 1);
		glutSolidCone(0.25, 0.75, 10, 10);
	}

	glPopMatrix();			// Restore glLookAt
}

void display_score() {
	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glDisable(GL_LIGHTING);
	glColor3f(0, 1.0, 0);
	glRasterPos3f(1.75, 3.25, 0);
	glEnable(GL_LIGHTING);

	char score_text[100];
	sprintf(score_text, "Score: %d", score);
	for (int i = 0; i < 100; ++i)
	{
		char c = score_text[i];
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
	}

	glPopMatrix();			// Restore glLookAt
}

void display_go() {
	glMatrixMode(GL_MODELVIEW);		// Setup model transformations
	glPushMatrix();			// Save glLookAt transformation

	glDisable(GL_LIGHTING);
	glColor3f(0, 1.0, 0);
	glRasterPos3f(-0.3, 3.25, 0);
	glEnable(GL_LIGHTING);

	char go[] = { 'G', 'A', 'M', 'E', ' ', 'O', 'V', 'E', 'R' };
	for (int i = 0; i < 10; ++i)
	{
		char c = go[i];
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
	}

	glPopMatrix();			// Restore glLookAt
}

//glutDusplayFunction
void display_things(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//lives
	display_lives();

	//score
	display_score();

	//display game over if no lives left
	if (lives == 0) {
		display_go();
	}

	//player
	if (respawn_timer == 0 && lives > 0) {
		display_player_ship();
	}
	else {
		respawn_timer--;
	}

	//bullets
	display_bullets();

	//asteroids
	for (int i = 0; i < num_asteroids; i++) {
		display_asteroid(i);
	}
	for (int i = 0; i < num_med_asteroids; i++) {
		display_med_asteroid(i);
	}
	for (int i = 0; i < num_small_asteroids; i++) {
		display_small_asteroid(i);
	}
	
	//aliens
	if (alien_timer == 0) {
		display_alien_ship();
	}
	else {
		alien_timer--;
		if (alien_timer == 0) {
			initialize_alien();
		}
	}

	//alien bullets
	display_alien_bullets();

	//explosions
	for (int i = 0; i <= explosion_counter; i++) {
		if (explosions[i].fuel > 0) {
			display_explosion(i);
		}
	}

	glutSwapBuffers();
}

void initialize_asteroid(int i) {

	if (!asteroid[i].initialized) {
		//set x, y, angle, dx, and dy
		int randLoc = (rand() % 4) + 1;
		if (randLoc == 1) {
			asteroid[i].x = 25.5;
			int rando = (rand() % 50) - 25;
			asteroid[i].y = rando;
		}
		else if (randLoc == 1) {
			asteroid[i].x = -25.5;
			int rando = (rand() % 50) - 25;
			asteroid[i].y = rando;
		}
		else if (randLoc == 1) {
			int rando = (rand() % 50) - 25;
			asteroid[i].x = rando;
			asteroid[i].y = 25.5;
		}
		else {
			int rando = (rand() % 50) - 25;
			asteroid[i].x = rando;
			asteroid[i].y = -25.5;
		}

		asteroid[i].angle = (rand() % 360) + 1;
		asteroid[i].dx = (cos(asteroid[i].angle) * (3.14 / 180)) * ASTEROID_SPEED;
		asteroid[i].dy = (sin(asteroid[i].angle) * (3.14 / 180)) * ASTEROID_SPEED;

		//reset axes
		asteroid[i].xAxis = false;
		asteroid[i].yAxis = false;
		asteroid[i].zAxis = false;
		//select random axis for rotation
		int randNum = (rand() % 3) + 1;
		if (randNum == 1) {
			asteroid[i].xAxis = true;
		}
		else if (randNum == 2) {
			asteroid[i].yAxis = true;
		}
		else {
			asteroid[i].zAxis = true;
		}

		asteroid[i].collision_radius = 2;
		asteroid[i].initialized = true;
		cur_asteroids++;
	}

}

void initialize_med_asteroid(int i, int j) {

	if (!m_asteroid[i].initialized) {
		//set x, y, angle, dx, and dy
		m_asteroid[i].x = asteroid[j].x / 0.75;
		m_asteroid[i].y = asteroid[j].y / 0.75;
		m_asteroid[i].angle = rand() % 360;
		m_asteroid[i].dx = (cos(m_asteroid[i].angle) * (3.14 / 180)) * M_ASTEROID_SPEED;
		m_asteroid[i].dy = (sin(m_asteroid[i].angle) * (3.14 / 180)) * M_ASTEROID_SPEED;

		//reset axes
		m_asteroid[i].xAxis = false;
		m_asteroid[i].yAxis = false;
		m_asteroid[i].zAxis = false;
		//select random axis for rotation
		int randNum = (rand() % 3) + 1;
		if (randNum == 1) {
			m_asteroid[i].xAxis = true;
		}
		else if (randNum == 2) {
			m_asteroid[i].yAxis = true;
		}
		else {
			m_asteroid[i].zAxis = true;
		}

		m_asteroid[i].collision_radius = 2;
		m_asteroid[i].initialized = true;
		cur_asteroids++;
	}

}

void initialize_small_asteroid(int i, int j) {

	if (!s_asteroid[i].initialized) {
		//set x, y, angle, dx, and dy
		s_asteroid[i].x = m_asteroid[j].x / 0.67;
		s_asteroid[i].y = m_asteroid[j].y / 0.67;
		s_asteroid[i].angle = rand() % 360;
		s_asteroid[i].dx = (cos(s_asteroid[i].angle) * (3.14 / 180)) * S_ASTEROID_SPEED;
		s_asteroid[i].dy = (sin(s_asteroid[i].angle) * (3.14 / 180)) * S_ASTEROID_SPEED;

		//reset axes
		s_asteroid[i].xAxis = false;
		s_asteroid[i].yAxis = false;
		s_asteroid[i].zAxis = false;
		//select random axis for rotation
		int randNum = (rand() % 3) + 1;
		if (randNum == 1) {
			s_asteroid[i].xAxis = true;
		}
		else if (randNum == 2) {
			s_asteroid[i].yAxis = true;
		}
		else {
			s_asteroid[i].zAxis = true;
		}

		s_asteroid[i].collision_radius = 2;
		s_asteroid[i].initialized = true;
		cur_asteroids++;
	}

}

void initialize_explosion(int x, int y) {

	int index = explosion_counter++;
	float angle = rand() % 360;

	for (int i = 0; i <= NUM_PARTICLES; i++)
	{
		explosions[index].particles[i].x = x;
		explosions[index].particles[i].y = y;
		explosions[index].particles[i].angle = (angle += 7.2);
		explosions[index].particles[i].speed = rand() % 5;
	}

	explosions[index].fuel = 10;

}

void initialize_alien() {
	int rando = (rand() % 2) + 1;
	if (rando == 0) {
		alien.x = -3.5;
	}
	else {
		alien.x = 3.5;
	}
	
	int rando2 = (rand() % 2) + 1;
	if (rando2 == 0) {
		alien.y = rand() % 3;
	}
	else {
		alien.y = rand() % 3 * -1;
	}
	alien.collision_radius = 0.1;
	alien.initialized = true;
}

void handle_menu( int ID ) {

	//  This routine handles popup menu selections
	switch( ID ) {
	case 0:				//New Game
		//reset player
		player.x = 0;
		player.y = 0;
		respawn_timer = 0;

		//reset alien
		alien.initialized = false;
		alien_bullet[0].initialized = false;
		alien_timer = 125;

		//reset small and medium asteroids
		med_counter = 0;
		small_counter = 0;

		//reset explosion counter
		explosion_counter = 0;

		//reset asteroids
		for (int i = 0; i < num_asteroids; i++) {
			asteroid[i].initialized = false;
		}

		//medium
		for (int i = 0; i < (num_med_asteroids * 2); i++) {
			m_asteroid[i].initialized = false;
		}

		//small
		for (int i = 0; i < (num_small_asteroids * 4); i++) {
			s_asteroid[i].initialized = false;
		}

		//re-initialize
		cur_max_asteroids = 2;
		for (int i = 0; i < cur_max_asteroids; i++) {
			initialize_asteroid(i);
		}

		//reset lives
		lives = 3;

		//reset score
		score = 0;
		break;
	case 1:				//Quit
		exit(0);
		break;
	}

}					// End routine handle_menu

void handle_mouse( int b, int s, int x, int y ) {

  //  This routine acts as a callback for GLUT mouse events
  //
  //  b:     Mouse button (left, middle, or right)
  //  s:     State (button down or button up)
  //  x, y:  Cursor position

  if ( s == GLUT_DOWN ) {		// Store button state if mouse down
    btn[ b ] = 1;
  } else {
    btn[ b ] = 0;
  }

  mouse_x = x;
  mouse_y = glutGet( GLUT_WINDOW_HEIGHT ) - y;
}					// End routine handle_mouse

//Timer function for rotating objects during runtime
void timer(int) {

	//update player ship location
	if (player_moving && respawn_timer == 0) {
		//accelerate
		if (ACCELERATION <= 1) {
			ACCELERATION += 0.15;
		}
		player.dx = cos(player.angle * (3.14 / 180)) * MAX_SPEED * ACCELERATION;
		player.dy = sin(player.angle * (3.14 / 180)) * MAX_SPEED * ACCELERATION;
		player.x += player.dx;
		player.y += player.dy;
	}
	else {
		//decelerate
		DRAG -= 0.025;
		if (DRAG >= 0 && player_start) {
			player.dx = cos(player.last_angle * (3.14 / 180)) * MAX_SPEED * DRAG;
			player.dy = sin(player.last_angle * (3.14 / 180)) * MAX_SPEED * DRAG;
			player.x += player.dx;
			player.y += player.dy;
		}
	}

	//check for collisions between player and alien
	//front of ship
	float d = pow(alien.x - ((player.x) + (cos(player.angle * (3.14 / 180)) / 3)), 2) + pow(alien.y - ((player.y) + (sin(player.angle * (3.14 / 180)) / 3)), 2);
	//back of ship
	float d2 = pow(alien.x - (player.x), 2) + pow(alien.y - (player.y), 2);

	if (d <= pow(alien.collision_radius, 2) || d2 <= pow(alien.collision_radius, 2)) {
		//reset player
		initialize_explosion(player.x, player.y);
		initialize_explosion(alien.x, alien.y);
		player.x = 0;
		player.y = 0;
		player.dx = 0;
		player.dy = 0;
		respawn_timer = 75;
		lives--;
		alien.initialized = false;
		alien_timer = 125;
	}

	//check for collisions between player and asteroids
	for (int k = 0; k < num_asteroids; k++) {
		if (asteroid[k].initialized) {
			//front of ship
			float d = pow(asteroid[k].x - ((player.x * 5) + (cos(player.angle * (3.14 / 180)) / 3)), 2) + pow(asteroid[k].y - ((player.y * 5) + (sin(player.angle * (3.14 / 180)) / 3)), 2);
			//back of ship
			float d2 = pow(asteroid[k].x - (player.x * 5), 2) + pow(asteroid[k].y - (player.y * 5), 2);
			if ((d <= pow(asteroid[k].collision_radius, 2) || d2 <= pow(asteroid[k].collision_radius, 2)) && respawn_timer == 0) {
				//reset player
				initialize_explosion(player.x, player.y);
				player.x = 0;
				player.y = 0;
				player.dx = 0;
				player.dy = 0;
				respawn_timer = 75;
				lives--;
				asteroid[k].initialized = false;
				initialize_explosion((asteroid[k].x / 5), (asteroid[k].y / 5));
				cur_asteroids--;

				//split into two medium asteroids
				initialize_med_asteroid(++med_counter, k);
				initialize_med_asteroid(++med_counter, k);
			}
		}
	}

	//check for collisions between player and medium asteroids
	for (int k = 0; k < num_med_asteroids; k++) {
		if (m_asteroid[k].initialized) {
			//front of ship
			float d = pow(m_asteroid[k].x - ((player.x * 7) + (cos(player.angle * (3.14 / 180)) / 3)), 2) + pow(m_asteroid[k].y - ((player.y * 7) + (sin(player.angle * (3.14 / 180)) / 3)), 2);
			//back of ship
			float d2 = pow(m_asteroid[k].x - (player.x * 7), 2) + pow(m_asteroid[k].y - (player.y * 7), 2);
			if ((d <= pow(m_asteroid[k].collision_radius, 2) || d2 <= pow(m_asteroid[k].collision_radius, 2)) && respawn_timer == 0) {
				//reset player
				initialize_explosion(player.x, player.y);
				player.x = 0;
				player.y = 0;
				player.dx = 0;
				player.dy = 0;
				respawn_timer = 75;
				lives--;
				m_asteroid[k].initialized = false;
				initialize_explosion((m_asteroid[k].x / 6.67), (m_asteroid[k].y / 6.67));
				cur_asteroids--;

				//split into two medium asteroids
				initialize_small_asteroid(++small_counter, k);
				initialize_small_asteroid(++small_counter, k);
			}
		}
	}

	//check for collisions between player and small asteroids
	for (int k = 0; k < num_small_asteroids; k++) {
		if (s_asteroid[k].initialized) {
			//front of ship
			float d = pow(s_asteroid[k].x - ((player.x * 10.5) + (cos(player.angle * (3.14 / 180)) / 3)), 2) + pow(s_asteroid[k].y - ((player.y * 10.5) + (sin(player.angle * (3.14 / 180)) / 3)), 2);
			//back of ship
			float d2 = pow(s_asteroid[k].x - (player.x * 10.5), 2) + pow(s_asteroid[k].y - (player.y * 10.5), 2);
			if ((d <= pow(s_asteroid[k].collision_radius, 2) || d2 <= pow(s_asteroid[k].collision_radius, 2)) && respawn_timer == 0) {
				//reset player
				initialize_explosion(player.x, player.y);
				player.x = 0;
				player.y = 0;
				player.dx = 0;
				player.dy = 0;
				respawn_timer = 75;
				lives--;
				s_asteroid[k].initialized = false;
				initialize_explosion((s_asteroid[k].x / 7.5), (s_asteroid[k].y / 7.5));
				cur_asteroids--;
			}
		}
	}

	//check to see if the player is off-screen
	//if so, wrap it
	if (player.x >= 3.5) {
		player.x = -3.5;
	}
	if (player.y >= 3.5) {
		player.y = -3.5;
	}
	if (player.x < -3.5) {
		player.x = 3.5;
	}
	if (player.y < -3.5) {
		player.y = 3.5;
	}

	//update ship orientation
	if (player_turning_right) {
		//turn right
		player.angle = player.angle - 5;
	}
	if (player_turning_left) {
		//turn left
		player.angle = player.angle + 5;
	}

	//update bullet location
	for (int i = 0; i < 4; i++) {
		if (bullet[i].initialized) {
			bullet[i].dx = cos(bullet[i].angle * (3.14 / 180)) * BULLET_SPEED;
			bullet[i].dy = sin(bullet[i].angle * (3.14 / 180)) * BULLET_SPEED;
			bullet[i].x += bullet[i].dx;
			bullet[i].y += bullet[i].dy;

			//check for asteroid collisions
			for (int k = 0; k < num_asteroids; k++) {
				if (asteroid[k].initialized) {
					float d = pow(asteroid[k].x - (bullet[i].x * 5), 2) + pow(asteroid[k].y - (bullet[i].y * 5), 2);
					if (d <= pow(asteroid[k].collision_radius, 2)) {
						bullet[i].initialized = false;
						asteroid[k].initialized = false;
						initialize_explosion((asteroid[k].x / 5), (asteroid[k].y / 5));
						cur_asteroids--;
						score += 20;
						if (score >= one_up) {
							lives++;
							one_up += 10000;
						}

						//split into two medium asteroids
						initialize_med_asteroid(++med_counter, k);
						initialize_med_asteroid(++med_counter, k);
					}
				}
			}

			//check for medium asteroid collisions
			for (int k = 0; k < num_med_asteroids; k++) {
				if (m_asteroid[k].initialized) {
					float d = pow(m_asteroid[k].x - (bullet[i].x * 6.67), 2) + pow(m_asteroid[k].y - (bullet[i].y * 6.67), 2);
					if (d <= pow(m_asteroid[k].collision_radius, 2)) {
						bullet[i].initialized = false;
						m_asteroid[k].initialized = false;
						initialize_explosion((m_asteroid[k].x / 6.67), (m_asteroid[k].y / 6.67));
						cur_asteroids--;
						score += 50;
						if (score >= one_up) {
							lives++;
							one_up += 10000;
						}

						//split into two small asteroids
						initialize_small_asteroid(++small_counter, k);
						initialize_small_asteroid(++small_counter, k);
					}
				}
			}

			//check for small asteroid collisions
			for (int k = 0; k < num_small_asteroids; k++) {
				if (s_asteroid[k].initialized) {
					float d = pow(s_asteroid[k].x - (bullet[i].x * 10), 2) + pow(s_asteroid[k].y - (bullet[i].y * 10), 2);
					if (d <= pow(s_asteroid[k].collision_radius, 2)) {
						bullet[i].initialized = false;
						s_asteroid[k].initialized = false;
						initialize_explosion((s_asteroid[k].x / 7.5), (s_asteroid[k].y / 7.5));
						cur_asteroids--;
						score += 100;
						if (score >= one_up) {
							lives++;
							one_up += 10000;
						}
					}
				}
			}

			//check for alien collisions
			float d = pow(alien.x - (bullet[i].x), 2) + pow(alien.y - (bullet[i].y), 2);
			if (d <= pow(alien.collision_radius * 2, 2) && alien.initialized) {
				bullet[i].initialized = false;
				alien.initialized = false;
				initialize_explosion(alien.x, alien.y);
				alien_timer = 125;
				score += 200;
				if (score >= one_up) {
					lives++;
					one_up += 10000;
				}
			}

			//check to see if the bullet is off-screen
			//if so, "destroy" it
			if (bullet[i].x >= 3.5 || bullet[i].y >= 3.5 || bullet[i].x <= -3.5 || bullet[i].y <= -3.5) {
				bullet[i].initialized = false;
			}
		}
	}

	//rotate alien ship
	alien.rotation += 2;

	//update alien ship position
	if (turn_timer == 0) {
		alien.angle = (rand() % 360) + 1;
		turn_timer = 150;
	}
	else {
		turn_timer--;
	}
	alien.dx = cos(alien.angle * (3.14 / 180)) * ALIEN_SPEED;
	alien.dy = sin(alien.angle * (3.14 / 180)) * ALIEN_SPEED;
	alien.x += alien.dx;
	alien.y += alien.dy;

	//check for collisions between alien and asteroids
	for (int k = 0; k < num_asteroids; k++) {
		if (asteroid[k].initialized) {
			float d2 = pow(asteroid[k].x - (alien.x * 5), 2) + pow(asteroid[k].y - (alien.y * 5), 2);
			if (d2 <= pow(asteroid[k].collision_radius, 2) && alien.initialized) {
				//reset alien
				initialize_explosion(alien.x, alien.y);
				alien.initialized = false;
				alien_timer = 125;
				asteroid[k].initialized = false;
				initialize_explosion((asteroid[k].x / 5), (asteroid[k].y / 5));
				cur_asteroids--;

				//split into two medium asteroids
				initialize_med_asteroid(++med_counter, k);
				initialize_med_asteroid(++med_counter, k);
			}
		}
	}

	//check for collisions between alien and medium asteroids
	for (int k = 0; k < num_med_asteroids; k++) {
		if (m_asteroid[k].initialized) {
			//front of ship
			float d = pow(m_asteroid[k].x - (alien.x * 7), 2) + pow(m_asteroid[k].y - (alien.y * 7), 2);
			if (d <= pow(m_asteroid[k].collision_radius, 2) && alien.initialized) {
				//reset alien
				initialize_explosion(alien.x, alien.y);
				alien.initialized = false;
				alien_timer = 125;
				m_asteroid[k].initialized = false;
				initialize_explosion((m_asteroid[k].x / 6.67), (m_asteroid[k].y / 6.67));
				cur_asteroids--;

				//split into two medium asteroids
				initialize_small_asteroid(++small_counter, k);
				initialize_small_asteroid(++small_counter, k);
			}
		}
	}

	//check for collisions between alien and small asteroids
	for (int k = 0; k < num_small_asteroids; k++) {
		if (s_asteroid[k].initialized) {
			//front of ship
			float d = pow(s_asteroid[k].x - (alien.x * 10.5), 2) + pow(s_asteroid[k].y - (alien.y * 10.5), 2);
			if (d <= pow(s_asteroid[k].collision_radius, 2) && alien.initialized) {
				//reset alien
				initialize_explosion(alien.x, alien.y);
				alien.initialized = false;
				alien_timer = 125;
				s_asteroid[k].initialized = false;
				initialize_explosion((s_asteroid[k].x / 7.5), (s_asteroid[k].y / 7.5));
				cur_asteroids--;
			}
		}
	}

	//check to see if the player is off-screen
	//if so, wrap it
	if (alien.x >= 3.5) {
		alien.x = -3.5;
	}
	if (alien.y >= 3.5) {
		alien.y = -3.5;
	}
	if (alien.x < -3.5) {
		alien.x = 3.5;
	}
	if (alien.y < -3.5) {
		alien.y = 3.5;
	}

	//alien shoot
	if (shoot_timer > 0) {
		shoot_timer--;
	}
	else {
		if (!alien_bullet[0].initialized && player_start && alien.initialized && shoot_timer == 0) {
			alien_bullet[0].x = alien.x;
			alien_bullet[0].y = alien.y;
			alien_bullet[0].angle = (rand() % 360) + 1;
			alien_bullet[0].initialized = true;
			shoot_timer = 50;
		}
	}

	//update alien bullet
	if (alien_bullet[0].initialized) {
		alien_bullet[0].dx = cos(alien_bullet[0].angle * (3.14 / 180)) * BULLET_SPEED;
		alien_bullet[0].dy = sin(alien_bullet[0].angle * (3.14 / 180)) * BULLET_SPEED;
		alien_bullet[0].x += alien_bullet[0].dx;
		alien_bullet[0].y += alien_bullet[0].dy;

		//check for asteroid collisions
		for (int k = 0; k < num_asteroids; k++) {
			if (asteroid[k].initialized) {
				float d = pow(asteroid[k].x - (alien_bullet[0].x * 5), 2) + pow(asteroid[k].y - (alien_bullet[0].y * 5), 2);
				if (d <= pow(asteroid[k].collision_radius, 2)) {
					alien_bullet[0].initialized = false;
					asteroid[k].initialized = false;
					initialize_explosion((asteroid[k].x / 5), (asteroid[k].y / 5));
					cur_asteroids--;

					//split into two medium asteroids
					initialize_med_asteroid(++med_counter, k);
					initialize_med_asteroid(++med_counter, k);
				}
			}
		}

		//check for medium asteroid collisions
		for (int k = 0; k < num_med_asteroids; k++) {
			if (m_asteroid[k].initialized) {
				float d = pow(m_asteroid[k].x - (alien_bullet[0].x * 6.67), 2) + pow(m_asteroid[k].y - (alien_bullet[0].y * 6.67), 2);
				if (d <= pow(m_asteroid[k].collision_radius, 2)) {
					alien_bullet[0].initialized = false;
					m_asteroid[k].initialized = false;
					initialize_explosion((m_asteroid[k].x / 6.67), (m_asteroid[k].y / 6.67));
					cur_asteroids--;

					//split into two small asteroids
					initialize_small_asteroid(++small_counter, k);
					initialize_small_asteroid(++small_counter, k);
				}
			}
		}

		//check for small asteroid collisions
		for (int k = 0; k < num_small_asteroids; k++) {
			if (s_asteroid[k].initialized) {
				float d = pow(s_asteroid[k].x - (alien_bullet[0].x * 10), 2) + pow(s_asteroid[k].y - (alien_bullet[0].y * 10), 2);
				if (d <= pow(s_asteroid[k].collision_radius, 2)) {
					alien_bullet[0].initialized = false;
					s_asteroid[k].initialized = false;
					initialize_explosion((s_asteroid[k].x / 7.5), (s_asteroid[k].y / 7.5));
					cur_asteroids--;
				}
			}
		}

		//check for collisions between player and alien bullet
		//front of ship
		float d = pow(alien_bullet[0].x - ((player.x) + (cos(player.angle * (3.14 / 180)) / 3)), 2) + pow(alien_bullet[0].y - ((player.y) + (sin(player.angle * (3.14 / 180)) / 3)), 2);
		//back of ship
		float d2 = pow(alien_bullet[0].x - (player.x), 2) + pow(alien_bullet[0].y - (player.y), 2);

		if ((d <= pow(alien.collision_radius, 2) || d2 <= pow(alien.collision_radius, 2)) && respawn_timer == 0) {
			//reset player
			initialize_explosion(player.x, player.y);
			player.x = 0;
			player.y = 0;
			player.dx = 0;
			player.dy = 0;
			respawn_timer = 75;
			lives--;
			alien_bullet[0].initialized = false;
		}

		//check to see if the bullet is off-screen
		//if so, "destroy" it
		if (alien_bullet[0].x >= 3.5 || alien_bullet[0].y >= 3.5 || alien_bullet[0].x <= -3.5 || alien_bullet[0].y <= -3.5) {
			alien_bullet[0].initialized = false;
		}
	}

	//update explosion particles
	for (int j = 0; j <= explosion_counter; j++) {
		if (explosions[j].fuel > 0)
		{
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				explosions[j].particles[i].dx = cos(explosions[j].particles[i].angle * (3.14 / 180)) * (explosions[j].particles[i].speed / 50);
				explosions[j].particles[i].dy = sin(explosions[j].particles[i].angle * (3.14 / 180)) * (explosions[j].particles[i].speed / 50);
				explosions[j].particles[i].x += explosions[j].particles[i].dx;
				explosions[j].particles[i].y += explosions[j].particles[i].dy;
			}
			explosions[j].fuel--;
		}
	}

	//update asteroid location and angle
	for (int i = 0; i < num_asteroids; i++) {
		if (asteroid[i].initialized) {
			asteroid[i].x += asteroid[i].dx;
			asteroid[i].y += asteroid[i].dy;
			asteroid[i].rotation += 1;

			//check to see if the asteroid is off-screen
			//if so, wrap it
			if (asteroid[i].x >= 20.5) {
				asteroid[i].x = -20.5;
			}
			if (asteroid[i].y >= 20.5) {
				asteroid[i].y = -20.5;
			}
			if (asteroid[i].x < -20.5) {
				asteroid[i].x = 20.5;
			}
			if (asteroid[i].y < -20.5) {
				asteroid[i].y = 20.5;
			}
		}
	}

	//udpate medium asteroid
	for (int i = 0; i < num_med_asteroids; i++) {
		if (m_asteroid[i].initialized) {
			m_asteroid[i].x += m_asteroid[i].dx;
			m_asteroid[i].y += m_asteroid[i].dy;
			m_asteroid[i].rotation += 1;

			//check to see if the asteroid is off-screen
			//if so, wrap it
			if (m_asteroid[i].x >= 25) {
				m_asteroid[i].x = -25;
			}
			if (m_asteroid[i].y >= 25) {
				m_asteroid[i].y = -25;
			}
			if (m_asteroid[i].x < -25) {
				m_asteroid[i].x = 25;
			}
			if (m_asteroid[i].y < -25) {
				m_asteroid[i].y = 25;
			}
		}
	}

	//update small asteroid
	for (int i = 0; i < num_small_asteroids; i++) {
		if (s_asteroid[i].initialized) {
			s_asteroid[i].x += s_asteroid[i].dx;
			s_asteroid[i].y += s_asteroid[i].dy;
			s_asteroid[i].rotation += 1;

			//check to see if the asteroid is off-screen
			//if so, wrap it
			if (s_asteroid[i].x >= 36.5) {
				s_asteroid[i].x = -36.5;
			}
			if (s_asteroid[i].y >= 36.5) {
				s_asteroid[i].y = -36.5;
			}
			if (s_asteroid[i].x < -36.5) {
				s_asteroid[i].x = 36.5;
			}
			if (s_asteroid[i].y < -36.5) {
				s_asteroid[i].y = 36.5;
			}
		}
	}

	//check to see if we need to reset screen
	if (cur_asteroids == 0) {
		//reset player
		player.x = 0;
		player.y = 0;

		//reset alien
		alien.initialized = false;
		alien_bullet[0].initialized = false;
		alien_timer = 125;

		//reset small and medium asteroids
		med_counter = 0;
		small_counter = 0;

		//reset explosion counter
		explosion_counter = 0;

		//increment max number of asteroids (if less than 6)
		//create that many large asteroids
		if (cur_max_asteroids < 6) {
			cur_max_asteroids++;
		}
		
		for (int i = 0; i < cur_max_asteroids; i++) {
			initialize_asteroid(i);
		}
	}
	
	glutPostRedisplay();
	glutTimerFunc(25.0, timer, 0);

}

//Function to handle rotating ship with arrow keys
void specialKeyboard(int key, int x, int y) {

	switch (key)
	{
	case GLUT_KEY_RIGHT:
		game_object temp[6];
		for (int i = 0; i < 6; i++) {
			temp[i] = asteroid[i];
		}
		player_turning_right = true;
		break;
	case GLUT_KEY_LEFT:
		player_turning_left = true;
		break;
	}

	glutPostRedisplay();

}

void specialKeyboardUp(int key, int x, int y) {

	switch (key)
	{
	case GLUT_KEY_RIGHT:
		player_turning_right = false;
		break;
	case GLUT_KEY_LEFT:
		player_turning_left = false;
		break;
	}

	glutPostRedisplay();

}

//Function to handle player ship acceleration and shooting
void keyboard(unsigned char key, int x, int y) {

	switch (key)
	{
	case 'x':
		//start moving
		player_moving = true;
		player_start = true;
		DRAG = 1;
		break;
	case 'z':
		//shoot
		if (respawn_timer == 0) {
			for (int i = 0; i < 4; i++) {
				if (!bullet[i].initialized) {
					bullet[i].x = player.x + (cos(player.angle * (3.14 / 180)) / 3);
					bullet[i].y = player.y + (sin(player.angle * (3.14 / 180)) / 3);
					bullet[i].angle = player.angle;
					bullet[i].initialized = true;
					break;
				}
			}
		}
		break;
	}

	glutPostRedisplay();

}

void keyboardUp(unsigned char key, int x, int y) {

	switch (key)
	{
	case 'x':
		//stop moving
		player_moving = false;
		player.last_angle = player.angle;
		ACCELERATION = 0;
		break;
	}

	glutPostRedisplay();

}

void lighting(void) {
	
	//  Enable Light 0 and GL lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	float  amb[] = { 0.5, 0.5, 0.5, 1 };	// Ambient material property
	float  lt_dif[] = { 0.5, 0.5, 0.5, 1 };	// Ambient light property
	float  lt_pos[] = {4, 0, 8, 1};  //Light position

	//  Set default ambient light in scene
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	//  Set Light 0 position, ambient, diffuse, specular intensities
	glLightfv(GL_LIGHT5, GL_POSITION, lt_pos);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, lt_dif);

}

void main( int argc, char *argv[] ) {

  glutInit( &argc, argv );		// Initialize GLUT
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize( 800, 800 );
  glutCreateWindow( "GLUT/XP Demo" );

  glutDisplayFunc( display_things );	// Setup GLUT callbacks
  glutMouseFunc( handle_mouse );

  glutCreateMenu( handle_menu );	// Setup GLUT popup menu
  glutAddMenuEntry("New Game", 0);
  glutAddMenuEntry( "Quit", 1 );
  glutAttachMenu( GLUT_RIGHT_BUTTON );

  glDisable(GL_CULL_FACE);

  glMatrixMode( GL_PROJECTION );	// Setup perspective projection
  glLoadIdentity();
  gluPerspective( 70, 1, 1, 40 );

  glMatrixMode( GL_MODELVIEW );		// Setup model transformations
  glLoadIdentity();
  gluLookAt( 0, 0, 5, 0, 0, -1, 0, 1, 0 );

  glShadeModel(GL_FLAT);		// Flat shading
  glEnable(GL_NORMALIZE);		// Normalize normals

  glClearDepth( 1.0 );			// Setup background colour
  glClearColor( 0, 0, 0, 0 );
  glEnable( GL_DEPTH_TEST );

  initialize_alien();

  lighting();

  // Return to main loop on window close
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );

  //use timer function for x-rotation during runtime 
  glutTimerFunc(25.0, timer, 0);

  //handle keybord presses (and releases)
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutSpecialFunc(specialKeyboard);
  glutSpecialUpFunc(specialKeyboardUp);

  glutMainLoop();			// Enter GLUT main loop

}