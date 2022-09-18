
//========================================================================
//
// Tetris
//
// This is based on example program splitview.c from the GLFW library
//
//========================================================================

// OpenGL
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <linmath.h>

//****************

// Standard
#include <array>
#include <limits>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

// 3P
#include <fmt/core.h>
#include <lodepng.h>

//========================================================================
// Global variables
//========================================================================

// OpenGL related

// Mouse position
double xpos = 0, ypos = 0;

// Window size
int width, height;

// Active view: 0 = none, 1 = upper left, 2 = upper right, 3 = lower left,
// 4 = lower right.  TODO: obsolete
int active_view = 0;

// Rotation around each axis
int rot_x = 0, rot_y = 0, rot_z = 0;

// Times
double t0, t, dt;

//****************

// Non-OpenGL

std::string me = "Tetris";

// Tetris world size, in [-WXH, WXH] x [-WY, 0].  TODO: these should be ints
const float WX = 20.0f, WY = 30.0f;
const float WXH = 0.5f * WX;

const float XMIN = -WXH, XMAX = WXH, YMIN = -WY, YMAX = 0;

// Number of cells (not points)
const int NX = (int) (XMAX - XMIN + 1);
const int NY = (int) (YMAX - YMIN + 1);
//constexpr int NX = (int) ceil(XMAX - XMIN);  // ceil is not compile-time const?
//constexpr int NY = (int) ceil(YMAX - YMIN);

const uint8_t NROT = 4;
const float ROTDEG = 360.f / NROT;

// Downward piece speed, units per second
float speed = 5.f;//0.5f;

// Active piece index
int64_t ip = -1;

//****************

// TODO: add runtime option for this?
bool enable_texture = !true;

const unsigned F_TEX_WIDTH  = 16;  // Floor texture dimensions
const unsigned F_TEX_HEIGHT = 16;

// Texture object IDs
std::vector<GLuint> tex_ids;

// TODO: check that this is at least as big as PieceType
const std::vector<std::string> TEX_FILES =
{
	"res/textures/hptt/Bark_01_2048.png",
	"res/textures/hptt/Brick_01_2048.png",
	"res/textures/hptt/Grass_02_2048.png",
	"res/textures/hptt/Rock_01_2048.png",
	"res/textures/hptt/Tile_01_2048.png",
	"res/textures/hptt/Wood_01_2048.png",
	"res/textures/hptt/Wood_02_2048.png",
	"res/textures/hptt/Grass_01_2048.png",
	"res/textures/hptt/Leaves_01_2048.png",
	//"res/textures/hptt/Leaves_01_2048_alpha.png",
	"res/textures/hptt/Dirt_01_2048.png"
};

//========================================================================

void log(const std::string& str, std::FILE* f = stdout)
{
	fmt::print(f, me + ": " + str + "\n");
	fflush(f);
}

void logerr(const std::string& str)
{
	log(str, stderr);
}

//========================================================================

void drawBlock()
{
	// Draw a 1 unit block with a corner at the origin
	//
	// TODO: load an asset with rounded corners?

	// Hack z-fighting by drawing blocks slightly smaller than 1 unit.  It's
	// noticeble on blocks in single pieces, not just neighboring pieces.
	float u = 0.95f; //1;
	float z = 1 - u;

	// Texture max (1 for full texture on every face)
	float t = 0.5f;

	static GLuint block_list = 0;
	if (!block_list)
	{
		// Start recording displaylist
		block_list = glGenLists(1);
		glNewList(block_list, GL_COMPILE_AND_EXECUTE);

		//glBegin(GL_QUAD_STRIP);
		glBegin(GL_QUADS);

			// Both the CW/CCW ordering of vertices and the normal are
			// important.  The CW ordering determines backface culling, while
			// the normal determines lighting

			// TODO: wrap texture coords around block instead of using full 0,1
			// range on each face

			// quad 1
			glNormal3f( 1,  0,  0);
			glTexCoord2f(0, t);
			glVertex3f(u, z, u);
			glTexCoord2f(t, t);
			glVertex3f(u, u, u);
			glTexCoord2f(t, 0);
			glVertex3f(u, u, z);
			glTexCoord2f(0, 0);
			glVertex3f(u, z, z);

			// quad 2
			glNormal3f( 0,  0,  1);
			glTexCoord2f(0, t);
			glVertex3f(z, u, u);
			glTexCoord2f(t, t);
			glVertex3f(u, u, u);
			glTexCoord2f(t, 0);
			glVertex3f(u, z, u);
			glTexCoord2f(0, 0);
			glVertex3f(z, z, u);

			// quad 3
			glNormal3f(-1,  0,  0);
			glTexCoord2f(0, t);
			glVertex3f(z, z, z);
			glTexCoord2f(t, t);
			glVertex3f(z, u, z);
			glTexCoord2f(t, 0);
			glVertex3f(z, u, u);
			glTexCoord2f(0, 0);
			glVertex3f(z, z, u);

			// quad 4
			glNormal3f( 0,  0, -1);
			glTexCoord2f(0, t);
			glVertex3f(z, z, z);
			glTexCoord2f(t, t);
			glVertex3f(u, z, z);
			glTexCoord2f(t, 0);
			glVertex3f(u, u, z);
			glTexCoord2f(0, 0);
			glVertex3f(z, u, z);

			// quad 5
			glNormal3f( 0,  1,  0);
			glTexCoord2f(0, t);
			glVertex3f(u, u, z);
			glTexCoord2f(t, t);
			glVertex3f(u, u, u);
			glTexCoord2f(t, 0);
			glVertex3f(z, u, u);
			glTexCoord2f(0, 0);
			glVertex3f(z, u, z);

			// quad 6
			glNormal3f( 0, -1,  0);
			glTexCoord2f(0, t);
			glVertex3f(z, z, z);
			glTexCoord2f(t, t);
			glVertex3f(z, z, u);
			glTexCoord2f(t, 0);
			glVertex3f(u, z, u);
			glTexCoord2f(0, 0);
			glVertex3f(u, z, z);

		glEnd();

		// Stop recording displaylist
		glEndList();
	}
	else
	{
		// Playback displaylist
		glCallList(block_list);
	}
}

//========================================================================

// Tetris pieces are shaped (roughly) like these letters.  G is like uppercase
// gamma (reverse L).
enum PieceType {I, L, O, S, G, Z, T, NTYPES};

// Define each piece in terms of an array of xy translations of each block
// relative to the piece's center.  Order of vector components must be the same
// as in the PieceType enum.
const std::vector< std::vector<float> > BLOCKS =
	{
		{ // I
			-0.5, -2,
			-0.5, -1,
			-0.5,  0,
			-0.5,  1
		},
		{ // L
			-1,  0.5,
			-1, -0.5,
			-1, -1.5,
			 0, -1.5
		},
		{ // O
			 0,  0,
			-1,  0,
			-1, -1,
			 0, -1
		},
		{ // S
			-1.5, -1,
			-0.5, -1,
			-0.5,  0,
			 0.5,  0
		},
		{ // G (L mirror)
			 0,  0.5,
			 0, -0.5,
			 0, -1.5,
			-1, -1.5
		},
		{ // Z (S mirror)
			 0.5, -1,
			-0.5, -1,
			-0.5,  0,
			-1.5,  0
		},
		{ // T
			-1.5, -1,
			-0.5, -1,
			 0.5, -1,
			-0.5,  0
		}
	};

const std::vector< std::vector<GLfloat> > COLORS =
	{

		// TODO: find a nice 7 color palette

		// Palette from:  https://colorswall.com/palette/175417
		{0.992f, 0.965f, 0.596f, 1.f},
		{0.902f, 0.349f, 0.518f, 1.f},
		{0.573f, 0.176f, 0.490f, 1.f},
		{0.035f, 0.463f, 0.737f, 1.f},
		{0.463f, 0.733f, 0.376f, 1.f},
		{0.945f, 0.541f, 0.518f, 1.f},
		{0.200f, 0.200f, 0.200f, 1.f}

		//// Palette from:  https://www.pinterest.com/pin/510103095296824882/
		//{0.263f, 0.259f, 0.278f, 1.f},
		//{0.461f, 0.453f, 0.500f, 1.f},
		//{0.996f, 0.737f, 0.408f, 1.f},
		//{0.988f, 0.859f, 0.549f, 1.f},
		//{0.122f, 0.584f, 0.537f, 1.f},
		//{0.255f, 0.753f, 0.710f, 1.f},
		//{0.000f, 0.000f, 0.000f, 1.f}

		//// Palette from:  https://www.schemecolor.com/six-pastels.php
		////
		//// Too pastel.  Maybe play with shininess or other settings?
		//{0.800f, 0.910f, 0.859f, 1.f},
		//{0.757f, 0.831f, 0.890f, 1.f},
		//{0.745f, 0.706f, 0.839f, 1.f},
		//{0.980f, 0.855f, 0.886f, 1.f},
		//{0.973f, 0.702f, 0.792f, 1.f},
		//{0.800f, 0.592f, 0.757f, 1.f},
		//{0.000f, 0.000f, 0.000f, 1.f}

	};

//========================================================================

class Piece
{
	public:
		float x, y;     // center position
		float  sx = 0;  // snap displacement (0 or 0.5)
		uint8_t r = 0;  // rotation state in [0, 3]
		PieceType t;

		void decompose();
		void move(float dx, float dy, bool key_initiated);
		void getBlock(int i, float& bx, float& by);
		void getMin(float& xmin, float& ymin);
		std::vector<float> getCenters();
		void snapx();
};

//std::vector<Piece> pieces;
Piece piece;

void newPiece()
{
	log("Starting newPiece()");
	Piece p;

	p.x = 0;
	p.y = 0;
	p.r = rand() % NROT;
	p.t = static_cast<PieceType>(rand() % NTYPES);

	p.snapx();

	//pieces.push_back(p);
	piece = p;

	ip++; // TODO unused

	//log(fmt::format("ip = {}", ip));
}

//========================================================================

void mat4x4gl_mul_vec4(vec4 r, GLfloat* const M, vec4 const v)
{
	// mat4x4_mul_vec4 takes mat4x4, and it's easier to re-implement with
	// GLfloat* than casting or copying
	//
	// TODO: consider adding glm dependency or some other lib?
	int i, j;
	for(j=0; j<4; ++j) {
		r[j] = 0.f;
		for(i=0; i<4; ++i)
			r[j] += M[4*i+j] * v[i];
	}
}

//========================================================================

std::array<std::array<PieceType, NY>, NX> blocks;

void Piece::decompose()
{
	// TODO: edit comment
	//
	// Decompose a piece into individual blocks and push them to a vec.  When
	// lines are eliminated later, individual blocks will have to be treated
	// instead of whole pieces.  Don't use a vector of pieces at all, since
	// there will only ever be 1 active piece.  The only thing that needs
	// a vector is settled blocks.
	//
	// Use an NX x NY array "blocks" of PieceType's to save state of settled
	// blocks, where NX = XMAX - XMIN, etc.  Set to NTYPES to indicate an empty
	// grid block, or another enum value to indicate an occupied grid block.

	log("Starting Piece::decompose()");
	for (int i = 0; i < BLOCKS[t].size() / 2; i++)
	{
		float xl, yl;
		getBlock(i, xl, yl);
		//log(fmt::format("xl yl = {} {}", xl, yl));

		int ix = (int) floor(xl - XMIN);
		int iy = (int) floor(yl - YMIN);
		log(fmt::format("ix iy = {} {}", ix, iy));

		blocks[ix][iy] = t;

		// TODO: with textures, the rotation state could also be saved to
		// a separate vec
	}
}

//========================================================================

void Piece::getBlock(int i, float& bx, float& by)
{
	// Get the center xy coordinates of block index i in this piece

	glPushMatrix();

	// Start from identity, not whatever's in the stack.  We only care about the
	// xy transformations here, not the projection view or other matrices.  This
	// differs from drawPieces
	glLoadIdentity();

	// Apply translation and rotation
	glTranslatef(x + sx, y, 0.0f);
	glRotatef(r * ROTDEG, 0.0f, 0.0f, 1.0f);

	// Get the transformation matrix
	GLfloat mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);

	// Add 0.5 to account for block corner to center
	vec4 vec = {BLOCKS[t][2*i] + 0.5f, BLOCKS[t][2*i+1] + 0.5f, 0, 1};

	// Apply transformation matrix to vec to calculate out
	vec4 out;
	mat4x4gl_mul_vec4(out, mat, vec);

	bx = out[0];
	by = out[1];
	//log(fmt::format("bx by = {} {}", bx, by));

	glPopMatrix();
}

//========================================================================

void Piece::getMin(float& xmin, float& ymin)
{
	// Get the min bounding xy coordinates of this piece
	//
	// TODO: leverage getCenters() and getCentersMin()

	//log("Starting Piece::getMin()");

	xmin = std::numeric_limits<float>::max();
	ymin = std::numeric_limits<float>::max();

	for (int i = 0; i < BLOCKS[t].size() / 2; i++)
	{
		float xl, yl;
		getBlock(i, xl, yl);
		xmin = std::min(xmin, xl);
		ymin = std::min(ymin, yl);

		//log(fmt::format("xl yl = {} {}", xl, yl));
	}

	// Center to min corner
	xmin -= 0.5f;
	ymin -= 0.5f;

	//log(fmt::format("xmin ymin = {} {}", xmin, ymin));
}

//========================================================================

void getCentersMin(std::vector<float>& xy, float& xmin, float& ymin)
{
	// Get the min bounds of center coordinates xy

	//log("Starting Piece::getCentersMin()");

	xmin = std::numeric_limits<float>::max();
	ymin = std::numeric_limits<float>::max();

	for (int i = 0; i < xy.size(); i += 2)
	{
		xmin = std::min(xmin, xy[i+0]);
		ymin = std::min(ymin, xy[i+1]);

		//log(fmt::format("xl yl = {} {}", xl, yl));
	}

	// Center to min corner
	xmin -= 0.5f;
	ymin -= 0.5f;

	//log(fmt::format("xmin ymin = {} {}", xmin, ymin));
}

//========================================================================

void getCentersMax(std::vector<float>& xy, float& xmax, float& ymax)
{
	// Get the max bounds of center coordinates xy

	//log("Starting Piece::getCentersMax()");

	xmax = std::numeric_limits<float>::min();
	ymax = std::numeric_limits<float>::min();

	for (int i = 0; i < xy.size(); i += 2)
	{
		xmax = std::max(xmax, xy[i+0]);
		ymax = std::max(ymax, xy[i+1]);

		//log(fmt::format("xl yl = {} {}", xl, yl));
	}

	// Center to max corner
	xmax += 0.5f;
	ymax += 0.5f;

	//log(fmt::format("xmax ymax = {} {}", xmax, ymax));
}

//========================================================================

std::vector<float> Piece::getCenters()
{
	// Get the xy coordinates of the center of each block in this piece

	//log("Starting Piece::getCenters()");

	std::vector<float> xy;

	for (int i = 0; i < BLOCKS[t].size() / 2; i++)
	{
		float xl, yl;
		getBlock(i, xl, yl);
		xy.push_back(xl);
		xy.push_back(yl);

		//log(fmt::format("xl yl = {} {}", xl, yl));
	}

	//log(fmt::format("xmin ymin = {} {}", xmin, ymin));
	return xy;
}

//========================================================================

void Piece::snapx()
{
	// Align to grid in x direction.  Pieces with odd dimensions have a center
	// that is not grid aligned, so this needs to be applied when rotating

	//log("Starting snapx()");

	sx = 0.f;

	float xl, yl;
	getMin(xl, yl);
	//if (abs((fmod(abs(xl), 1)) - 0.5) > 0.1)  // 0.1 tol may not be sufficient for snapy
	//if (abs(fmod(xl, 1)) < 0.1)  // 0.1 tol may not be sufficient for snapy
	if ((int) round((abs(xl) * 2)) % 2 == 1)
		sx = 0.5f;

	//log(fmt::format("snapx: sx = {}", sx));
}

//========================================================================

void Piece::move(float dx, float dy, bool key_initiated = true)
{
	// Backup position, try moving, check for collisions, and return to original
	// position if needed.  If the piece is bottomed-out, destroy it and make
	// a new piece at the top

	//log("Starting Piece::move()");

	auto x0 = x, y0 = y;

	//y -= speed * (float) dt;
	x += dx;
	y += dy;

	auto xy = getCenters();

	// Clamp based on y bound, not center
	float xl, yl;
	getCentersMin(xy, xl, yl);
	//log(fmt::format("y, yl, YMIN = {}, {}, {}", y, yl, YMIN));

	// A tolerance of 0.05 is small enough to not notice the piece bounce back
	// up after falling into a colision.  0.01 seems to work, but is probably to
	// difficult to slip a piece under a ledge.  0.1 results in a noticeable
	// bounce
	double tol = 0.05;//0.2;

	// TODO: work on key repeat logic to make ledge slipping easier.  Don't rely
	// on OS repeat, just store our own hold/release state

	if (xl < XMIN - tol)
	{
		// TODO: previous rotation too, here and elsewhere
		x = x0;
		//x += XMIN - xl;
	}

	if (yl < YMIN - tol)
	{
		//y = y0;
		y += YMIN - yl;

		// Only make a new piece for collision in y dir.  Only downward motion
		// (either natural falling or down key) can settle a piece, not
		// rotations or x motion
		piece.decompose();
		newPiece();
		return;
	}

	getCentersMax(xy, xl, yl);
	if (xl > XMAX + tol)
	{
		//x += XMAX - xl;
		x = x0;
	}

	// There is no need to check ymax

	// Check for collisions with settled blocks
	bool collide = false;
	for (int ix = 0; ix < blocks.size(); ix++)
		for (int iy = 0; iy < blocks[ix].size(); iy++)
		{
			PieceType t = blocks[ix][iy];

			// Skip empty blocks
			if (t >= NTYPES) continue;

			//// old block coordinates
			//float x = ix + XMIN;
			//float y = iy + YMIN;

			//float ixb = (int) round(ix + XMIN);
			//float iyb = (int) round(iy + YMIN);
			//float xb = ix + XMIN + 0.5f;
			//float yb = iy + YMIN + 0.5f;

			// TODO: move invariant x outside of y loop
			double xblo = ix + XMIN + 0 + tol;
			double yblo = iy + YMIN + 0 + 0.8;//tol; // large tol here facilitates ledge slipping
			double xbhi = ix + XMIN + 1 - tol;
			double ybhi = iy + YMIN + 1 - tol;

			for (int i = 0; i < xy.size(); i += 2)
			{
				// This piece's block center coordinates
				xl = xy[i+0];
				yl = xy[i+1];

				// Range.  TODO: double or int?  tol?

				//int ixlo = (int) floor(xl - 0.5f);
				//int ixhi = (int) floor/*ceil*/ (xl + 0.5f);
				//int iylo = (int) floor(yl - 0.5f);
				//int iyhi = (int) floor/*ceil*/ (yl + 0.5f);

				//int ixlo = (int) floor(xl - 0.5f + tol);
				//int ixhi = (int) floor(xl + 0.5f - tol);
				//int iylo = (int) floor(yl - 0.5f + tol);
				//int iyhi = (int) floor(yl + 0.5f - tol);
				//collide = ixlo <= ixb && ixb <= ixhi
				//       && iylo <= iyb && iyb <= iyhi;
				//collide = xlo <= xb && xb <= xhi
				//       && ylo <= yb && yb <= yhi;

				double xlo = xl - 0.5;// + tol;
				double xhi = xl + 0.5;// - tol;
				double ylo = yl - 0.5;// + tol;
				double yhi = yl + 0.5;// - tol;

				collide = !((xhi < xblo || xlo > xbhi) 
				         || (yhi < yblo || ylo > ybhi));

				if (collide) goto endloop;
			}
		}
	endloop:

	if (collide)
	{
		x = x0;
		y = y0;

		// TODO: check collide again now after reseting y.  don't decompose if
		// it's clear.  Better yet, only allow decomposition for -dt based
		// moves, not arrow-key initiated moves

		// TODO: reset rotation, and maybe decompose and newPiece

		// Allowing a badly-timed key-initiated decomposition results in a piece
		// ending up 1 block higher than it should be
		if (dy < 0 && !key_initiated)
		{
			piece.decompose();
			newPiece();
			return;
		}
	}

}

//========================================================================

void drawPiece(std::vector<float> b)
{
	for (int i = 0; i < b.size(); i += 2)
	{
		glTranslatef( b[i],  b[i+1], 0.f); // push
		drawBlock();
		glTranslatef(-b[i], -b[i+1], 0.f); // pop
	}
}

//========================================================================

void drawPieces()
{
	// TODO: eliminate func? there's only 1 piece.  although, we may want to
	// display a preview of the next piece before it becomes active

	//for (auto p: pieces)
	Piece p = piece;
	{
		glPushMatrix();

		glTranslatef(p.x + p.sx, p.y, 0.0f);

		// Alternatively, could use mat4x4_rotate_Z()
		glRotatef(p.r * ROTDEG, 0.0f, 0.0f, 1.0f);

		if (enable_texture)
		{
			glEnable(GL_TEXTURE_2D);
			const GLfloat base_color[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
			glMaterialfv(GL_FRONT, GL_DIFFUSE, base_color);
			glBindTexture(GL_TEXTURE_2D, tex_ids[p.t]);
		}
		else
		{
			// .data() returns a pointer to a C array
			glMaterialfv(GL_FRONT, GL_DIFFUSE, COLORS[p.t].data());
		}

		drawPiece(BLOCKS[p.t]);

		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}
}

//========================================================================

void drawBlocks()
{
	//log("Starting drawBlocks()");
	//log(fmt::format("size blocks outer = {}", blocks   .size()));
	//log(fmt::format("size blocks inner = {}", blocks[0].size()));

	for (int ix = 0; ix < blocks.size(); ix++)
		for (int iy = 0; iy < blocks[ix].size(); iy++)
		{
			PieceType t = blocks[ix][iy];

			// Skip empty blocks
			if (t >= NTYPES) continue;

			float x = ix + XMIN;
			float y = iy + YMIN;

			glPushMatrix();

			glTranslatef(x, y, 0.0f);

			if (enable_texture)
			{
				glEnable(GL_TEXTURE_2D);
				const GLfloat base_color[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
				glMaterialfv(GL_FRONT, GL_DIFFUSE, base_color);
				glBindTexture(GL_TEXTURE_2D, tex_ids[t]);
			}
			else
			{
				// .data() returns a pointer to a C array
				glMaterialfv(GL_FRONT, GL_DIFFUSE, COLORS[t].data());
			}

			drawBlock();

			glDisable(GL_TEXTURE_2D);
			glPopMatrix();

		}
}

//========================================================================

void drawBoard()
{
	// Draw world boundary and vertical grid lines, because perspective makes it
	// hard to see where a piece will land

	// Line coloring only works with lighting disabled.  Re-enable later for
	// pieces
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	glColor3f(0.8f, 0.8f, 0.8f);

	glVertex3f(XMIN, YMIN, 0);
	glVertex3f(XMAX, YMIN, 0);

	glVertex3f(XMIN, YMAX, 0);
	glVertex3f(XMAX, YMAX, 0);

	const float GRID_SPACING = 4;
	for (float x = XMIN; x <= XMAX + 0.1f; x += GRID_SPACING)
	{
		glVertex3f(x, YMIN, 0);
		glVertex3f(x, YMAX, 0);
	}

	glEnd();
	glEnable(GL_LIGHTING);
}

//========================================================================

void drawScene()
{
	const GLfloat model_diffuse[4]  = {1.0f, 0.8f, 0.8f, 1.0f};
	const GLfloat model_specular[4] = {0.6f, 0.6f, 0.6f, 1.0f};
	const GLfloat model_shininess   = 20.0f;

	glPushMatrix();

	// Rotate the scene
	glRotatef((GLfloat) rot_x * 0.5f, 1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat) rot_y * 0.5f, 0.0f, 1.0f, 0.0f);
	glRotatef((GLfloat) rot_z * 0.5f, 0.0f, 0.0f, 1.0f);

	// Set model color (used for orthogonal views, lighting disabled)
	glColor4fv(model_diffuse);

	// Set model material (used for perspective view, lighting enabled)
	glMaterialfv(GL_FRONT, GL_DIFFUSE, model_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, model_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, model_shininess);

	drawBoard();
	drawPieces();
	drawBlocks();

	glPopMatrix();
}

//========================================================================

void drawAllViews()
{
	// Set light position based on world size
	const GLfloat light_position[4] = {1.5 * WXH, -0.5 * WY, WY, 1.0f};
	//const GLfloat light_position[4] = {0.0f, 8.0f, 8.0f, 1.0f};
	//const GLfloat light_position[4] = {0.0f, 0.0f, 40.0f, 1.0f};

	const GLfloat light_diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLfloat light_specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLfloat light_ambient[4]  = {0.2f, 0.2f, 0.3f, 1.0f};
	float aspect;
	mat4x4 view, projection;

	// Calculate aspect of window
	if (height > 0)
		aspect = (float) width / (float) height;
	else
		aspect = 1.f;

	// Clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// Gradient background (c.f. JeffIrwin/rubik-js)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(0.082f, 0.341f, 0.6f);
	glVertex2f(-1.0, 1.0);
	glColor3f(0.082f, 0.471f, 0.471f);
	glVertex2f(-1.0,-1.0);
	glColor3f(0.082f, 0.6f, 0.341f);
	glVertex2f(1.0,-1.0);
	glColor3f(0.082f, 0.471f, 0.471f);
	glVertex2f(1.0, 1.0);
	glEnd();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Use solid rendering
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Enable backface culling (faster rendering)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	// Setup perspective projection matrix
	glMatrixMode(GL_PROJECTION);
	mat4x4_perspective(projection,
					   65.f * (float) M_PI / 180.f,
					   aspect,
					   1.f, 50.f); // last arg (50) controls zFar culling
	glLoadMatrixf((const GLfloat*) projection);

	// Upper right view (PERSPECTIVE VIEW)
	glViewport(0, 0, width, height);
	glMatrixMode(GL_MODELVIEW);
	{
		vec3 up = {0.f, 1.f, 0.f};

		// eye z is fudged such that world is in FOV

		//// interesting perspective
		//vec3 eye = {0.5f * WXH, 0, 0.95f * WY};
		//vec3 center = {0.f, -0.5 * WY, 0.f};

		// easier to judge alignment (at least until I implement gridlines)
		vec3 eye = {0, -0.5 * WY, 0.95 * WY};
		vec3 center = {0, -0.5 * WY, 0};

		//vec3 eye = { 3.f, 1.5f, 4.f };
		//vec3 center = { 2.f, -1.f, 0.f };

		mat4x4_look_at( view, eye, center, up );
	}
	glLoadMatrixf((const GLfloat*) view);

	// Configure and enable light source 1
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	// Draw scene
	drawScene();

	// Disable lighting
	glDisable(GL_LIGHTING);

	// Disable face culling
	glDisable(GL_CULL_FACE);

	// Disable depth test
	glDisable(GL_DEPTH_TEST);
}

//========================================================================

void framebufferSizeFun(GLFWwindow* window, int w, int h)
{
	// Framebuffer size callback function

	width  = w;
	height = h > 0 ? h : 1;
}

//========================================================================

void windowRefreshFun(GLFWwindow* window)
{
	// Window refresh callback function
	drawAllViews();
	glfwSwapBuffers(window);
}

//========================================================================

void cursorPosFun(GLFWwindow* window, double x, double y)
{
	// Mouse position callback function

	int wnd_width, wnd_height, fb_width, fb_height;
	double scale;

	glfwGetWindowSize(window, &wnd_width, &wnd_height);
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	scale = (double) fb_width / (double) wnd_width;

	x *= scale;
	y *= scale;

	// TODO: apply a rotation like in temple-viewer

	// Depending on which view was selected, rotate around different axes
	switch (active_view)
	{
		case 1:
		case 2:
			rot_x += (int) (y - ypos);
			rot_z += (int) (x - xpos);
			break;
		case 3:
			rot_x += (int) (y - ypos);
			rot_y += (int) (x - xpos);
			break;
		case 4:
			rot_y += (int) (x - xpos);
			rot_z += (int) (y - ypos);
			break;
		default:
			// Do nothing for perspective view, or if no view is selected
			break;
	}

	// Remember cursor position
	xpos = x;
	ypos = y;
}

//========================================================================

void mouseButtonFun(GLFWwindow* window, int button, int action, int mods)
{
	// Mouse button callback function

	if ((button == GLFW_MOUSE_BUTTON_LEFT) && action == GLFW_PRESS)
	{
		// Detect which of the four views was clicked
		active_view = 1;
		if (xpos >= width / 2)
			active_view += 1;
		if (ypos >= height / 2)
			active_view += 2;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// Deselect any previously selected view
		active_view = 0;
	}
}

//========================================================================

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//log("Starting key_callback()");
	//log(fmt::format("key action = {} {}", key, action));

	//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	//	glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_LEFT)
			piece.move(-1,  0);
		else if (key == GLFW_KEY_RIGHT)
			piece.move( 1,  0);
		else if (key == GLFW_KEY_DOWN)
			piece.move( 0, -1);
		else if (key == GLFW_KEY_J)
		{
			// Rotate CCW
			//
			// TODO: make this a function that detects collisions.  Combine with
			// move() (extra arg for rot)?
			//
			// Add an extra NROT to prevent underflow.  Anyway for a 1-byte int
			// mod 4, it doesn't matter because 255%4 == 3%4
			piece.r = (piece.r + NROT + 1u) % NROT;
			piece.snapx();
			log(fmt::format("r = {}", piece.r));
		}
		else if (key == GLFW_KEY_K)
		{
			// Rotate CW
			piece.r = (piece.r + NROT - 1u) % NROT;
			piece.snapx();
			log(fmt::format("r = {}", piece.r));
		}
	}
}

//========================================================================

GLFWimage png2gimg(const std::string& filename)
{
	// Decode a PNG file and return a GLFWimage
	log("png2gimg: decoding " + filename);

	unsigned error, w, h;
	GLFWimage image;

	error = lodepng_decode32_file(&image.pixels, &w, &h, filename.c_str());
	//error = lodepng_decode24_file(&image.pixels, &w, &h, filename.c_str());
	if (error) logerr(fmt::format("Error {}: {}\n", error, lodepng_error_text(error)));

	// Cast unsigned to signed
	image.width  = w;
	image.height = h;

	return image;
}

//========================================================================

int main()
{
	log("");
	log("Starting main()");
	log("");

	GLFWwindow* window;

	// Initialise GLFW
	if (!glfwInit())
	{
		logerr("Error: Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	// Enable multisampling, whatever that means.  idk it looks better this way
	// :shrug:
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Open OpenGL window
	window = glfwCreateWindow(1280, 1280, me.c_str(), NULL, NULL);
	//window = glfwCreateWindow(1280, 1280, me.c_str(),
	//	glfwGetPrimaryMonitor(), NULL);

	// TODO: use path relative to runtime.  Copy to build dir in cmake, get
	// exe's own path at runtime (for Windows at least)
	glfwSetWindowIcon(window, 1, &png2gimg("res/icon.png"));

	// TODO: add cmd arg for position or maximize?
	glfwSetWindowPos(window, 100, 50);
	//glfwMaximizeWindow(window);

	if (!window)
	{
		logerr("Error: Failed to open GLFW window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Set callback functions
	glfwSetFramebufferSizeCallback(window, framebufferSizeFun);
	glfwSetWindowRefreshCallback(window, windowRefreshFun);
	glfwSetCursorPosCallback(window, cursorPosFun);
	glfwSetMouseButtonCallback(window, mouseButtonFun);
	glfwSetKeyCallback(window, key_callback);

	// Enable vsync
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1);

	if (GLAD_GL_ARB_multisample || GLAD_GL_VERSION_1_3)
		glEnable(GL_MULTISAMPLE_ARB);

	glfwGetFramebufferSize(window, &width, &height);
	framebufferSizeFun(window, width, height);

	//****************

	if (enable_texture)
	{
		// Load textures from resource files

		tex_ids.reserve(TEX_FILES.size());
		int i = 0;
		for (auto f: TEX_FILES)
		{
			GLFWimage tex = png2gimg(f);

			// Upload texture(s)

			glGenTextures(1, &tex_ids[i]);
			glBindTexture(GL_TEXTURE_2D, tex_ids[i]);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height,
					0, GL_RGBA, GL_UNSIGNED_BYTE, tex.pixels);

			// Mipmaps optimize hi-res images for display on small objects.

			// TODO: outside loop?
			//glGenerateMipmap(GL_TEXTURE_2D);

			i++;
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	//****************

	// Seed rng for piece generation
	srand((unsigned int) time(NULL));
	ip = -1;
	newPiece();

	log(fmt::format("NX NY = {} {}", NX, NY));

	// Mark all blocks as empty initially.  Is vec<vec> the right data struct
	// here?  It sure is a pain to initialize
	for (int ix = 0; ix < blocks.size(); ix++) //(auto col: blocks)
		for (int iy = 0; iy < blocks[ix].size(); iy++)//(auto block: col)
			blocks[ix][iy] = NTYPES;

	//log(fmt::format("enum = {} {} {} {} {} {}", I, L, O, S, G, Z));
	log("Starting main loop");

	// Main loop
	t0 = glfwGetTime();
	for (;;)
	{
		// Timing
		t = glfwGetTime();
		dt = t - t0;
		t0 = t;

		windowRefreshFun(window);

		// Wait for new events
		//glfwWaitEvents();
		glfwPollEvents();

		piece.move(0, -speed * (float) dt, false);

		// Check if the window should be closed
		if (glfwWindowShouldClose(window))
			break;
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	log("Exiting main() successfully\n");
	exit(EXIT_SUCCESS);
}

//========================================================================

