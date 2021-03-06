#include "Platform.hpp"
#include <cmath>

/**
 * Returns a value that is src incremented/decremented by inc towards goal
 * until goal is reached. Does not overshoot.
 */
float approach_by_increment(float goal, float src, float inc) {
    float newVal;

    if (src <= goal) {
        if (goal - src < inc) {
            newVal = goal;
        }
        else {
            newVal = src + inc;
        }
    }
    else if (goal - src > -inc) {
        newVal = goal;
    }
    else {
        newVal = src - inc;
    }

    return newVal;
}

/**
 * Creates a transform matrix on a variable passed in from given normals
 * and the object's position.
 */
void Platform::create_transform_from_normals() {
	mtxf_align_terrain_normal(transform, normal, pos, 0);
}

Surface const * Platform::find_floor(Mario* m) const {
	Surface const * floor = NULL;

	int16_t x = static_cast<int16_t>(static_cast<int>(m->pos[0]));
	int16_t y = static_cast<int16_t>(static_cast<int>(m->pos[1]));
	int16_t z = static_cast<int16_t>(static_cast<int>(m->pos[2]));

	for (int i = 0; i < 3; i++) {
		const Surface& surf = triangles[i];

		int16_t x1 = surf.vector1[0];
		int16_t z1 = surf.vector1[2];
		int16_t x2 = surf.vector2[0];
		int16_t z2 = surf.vector2[2];

		// Check that the point is within the triangle bounds.
		if ((z1 - z) * (x2 - x1) - (x1 - x) * (z2 - z1) < 0) {
			continue;
		}

		// To slightly save on computation time, set this later.
		int16_t x3 = surf.vector3[0];
		int16_t z3 = surf.vector3[2];

		if ((z2 - z) * (x3 - x2) - (x2 - x) * (z3 - z2) < 0) {
			continue;
		}
		if ((z3 - z) * (x1 - x3) - (x3 - x) * (z1 - z3) < 0) {
			continue;
		}

		float nx = normal[0];
		float ny = normal[1];
		float nz = normal[2];
		float oo = -(nx * x1 + ny * surf.vector1[1] + nz * z1);

		// Find the height of the floor at a given location.
		float height = -(x * nx + nz * z + oo) / ny;
		// Checks for floor interaction with a 78 unit buffer.
		if (y - (height + -78.0f) < 0.0f) {
			continue;
		}

		floor = &surf;
		break;
	}

	//! (Surface Cucking) Since only the first floor is returned and not the highest,
	//  higher floors can be "cucked" by lower floors.
	return floor;
}

void Platform::platform_logic(Mario* m) {
	float dx;
	float dy;
	float dz;
	float d;

	Vec3f dist;
	Vec3f posBeforeRotation;
	Vec3f posAfterRotation;

	//create_transform_from_normals();

	// Mario's position
	float mx;
	float my;
	float mz;

	mx = m->pos[0];
	my = m->pos[1];
	mz = m->pos[2];

	dist[0] = mx - -1945.0;
	dist[1] = my - -3225.0;
	dist[2] = mz - -715.0;
	linear_mtxf_mul_vec3f(posBeforeRotation, transform, dist);

	dx = mx - -1945.0;
	dy = 500.0f;
	dz = mz - -715.0;
	d = sqrtf(dx * dx + dy * dy + dz * dz);

	//! Always true since dy = 500, making d >= 500.
	if (d != 0.0f) {
		// Normalizing
		d = 1.0 / d;
		dx *= d;
		dy *= d;
		dz *= d;
	}
	else {
		dx = 0.0f;
		dy = 1.0f;
		dz = 0.0f;
	}

	/*
	if (o->oTiltingPyramidMarioOnPlatform == TRUE)
		marioOnPlatform++;

	o->oTiltingPyramidMarioOnPlatform = TRUE;*/

	// Approach the normals by 0.01f towards the new goal, then create a transform matrix and orient the object. 
	// Outside of the other conditionals since it needs to tilt regardless of whether Mario is on.
	normal[0] = approach_by_increment(dx, normal[0], 0.01f);
	normal[1] = approach_by_increment(dy, normal[1], 0.01f);
	normal[2] = approach_by_increment(dz, normal[2], 0.01f);
	create_transform_from_normals();

	//triangles[0].rotate(transform);
	//triangles[1].rotate(transform);
	//don't care about rotating the triangles after the displacement

	// pretty sure you can always assume if here, then mario is on the floor
	Surface const* floor = this->find_floor(m);

	// If Mario is on the platform, adjust his position for the platform tilt.
	if (floor) {
		linear_mtxf_mul_vec3f(posAfterRotation, transform, dist);
		mx += posAfterRotation[0] - posBeforeRotation[0];
		my += posAfterRotation[1] - posBeforeRotation[1];
		mz += posAfterRotation[2] - posBeforeRotation[2];
		m->pos[0] = mx;
		m->pos[1] = my;
		m->pos[2] = mz;
	}
}