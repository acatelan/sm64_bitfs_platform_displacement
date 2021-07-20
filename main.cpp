#include "Mario.h"
#include "Magic.h"
#include "Platform.h"
#include "vmath.h"
#include <cmath>
#include <mutex>

#ifdef _OPENMP
#include <omp.h> // This line won't add the library if you don't compile with -fopenmp option.
#ifdef _MSC_VER
// For Microsoft compiler
#define OMP_FOR __pragma(omp parallel for)
#else // assuming "__GNUC__" is defined
// For GCC compiler
#define OMP_FOR _Pragma("omp parallel for")

#endif
#else
#define omp_get_max_threads() 1
#define OMP_FOR

#endif

static std::mutex print_mutex;

using namespace std;

void brute_angles(Mario* m, Platform* plat, const Vec3f& m_pos, float spd, const Vec3f & normals, const Mat4& trans) {
	//iterate over hau instead of sticks
	for (int hau = 0; hau < 65535; hau += 16) {
		if (abs((short)(int)(m_pos[0] + gSineTable[(hau & 0xFFFF) >> 4] * normals[1] * (spd / 4.0f))) >= 8192) {
			continue;
		}
		m->set_pos(m_pos);
		m->speed = spd;


		if (m->ground_step(hau, normals[1]) == 0) { continue; }
		
		for (int i = 0; i < 3; i++) { plat->normal[i] = normals[i]; }

		if (!plat->find_floor(m)) { continue; }

		plat->platform_logic(m);

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) { plat->transform[i][j] = trans[i][j]; }
		}

		if (!check_inbounds(*m)) { continue; }

		if (m->pos[1] >= 3521 && m->pos[1] < 8192) {
			print_mutex.lock();
			if (m->pos[1] <= 3841) {
				printf("-------------------\nIDEAL SOLN\nBully spd: %f\nHau: %d\nPlatform normals: (%.9f, %.9f, %.9f)\nMario pos: (%.9f, %.9f, %.9f)\nMario start: (%.9f, %.9f, %.9f)\n",
					m->speed, hau, normals[0], normals[1], normals[2], m->pos[0], m->pos[1], m->pos[2], m_pos[0], m_pos[1], m_pos[2]);
			}
			else {
				printf("-------------------\nACCEPTABLE SOLN\nBully spd: %f\nHau: %d\nPlatform normals: (%.9f, %.9f, %.9f)\nMario pos: (%.9f, %.9f, %.9f)\nMario start: (%.9f, %.9f, %.9f)\n",
					m->speed, hau, normals[0], normals[1], normals[2], m->pos[0], m->pos[1], m->pos[2], m_pos[0], m_pos[1], m_pos[2]);
			}
			print_mutex.unlock();
		}
	}

	/*
	for (int16_t x = -128; x < 128; x++) {
		for (int16_t y = -128; y < 128; y++) {
			m->pos = m_pos;
			m->speed = spd;

			plat->normal = normals;

			yawmag = calc_intended_yawmag(x, y);
			int16_t intYaw = yawmag.first - (yawmag.first % 16);

			if (m->ground_step(x, y, plat->normal[1]) == 0) { continue; }

			plat->platform_logic(m);

			if (!check_inbounds(*m)) { continue; }

			if (m->pos[1] >= 3521) {
				if (m->pos[1] <= 3841) {
					printf("-------------------\nIDEAL SOLN\nBully spd: %f\nStick X: %d\nStick Y: %d\nPlatform normals: (%.9f, %.9f, %.9f)\nMario pos: (%.9f, %.9f, %.9f)\nMario start: (%.9f, %.9f, %.9f)\n",
						m->speed, x, y, normals[0], normals[1], normals[2], m->pos[0], m->pos[1], m->pos[2], m_pos[0], m_pos[1], m_pos[2]);
				}
				else if (m->pos[1] < 8192) {
					printf("-------------------\nACCEPTABLE SOLN\nBully spd: %f\nStick X: %d\nStick Y: %d\nPlatform normals: (%.9f, %.9f, %.9f)\nMario pos: (%.9f, %.9f, %.9f)\nMario start: (%.9f, %.9f, %.9f)\n",
						m->speed, x, y, normals[0], normals[1], normals[2], m->pos[0], m->pos[1], m->pos[2], m_pos[0], m_pos[1], m_pos[2]);
				}
			}
		}
	}*/
}

void brute_position(Mario* m, Platform* plat, float spd, const Vec3f& normals) {
	for (int i = 0; i < 3; i++) { plat->normal[i] = normals[i]; }

	plat->create_transform_from_normals();
	plat->triangles[0].rotate(plat->pos, plat->transform);
	plat->triangles[1].rotate(plat->pos, plat->transform);

	const Mat4& trans = plat->transform;
	const Vec2S& tri = plat->triangles;

	float max_x = max(plat->triangles[1].vector1[0], plat->triangles[1].vector3[0]);
	float min_x = min(plat->triangles[1].vector1[0], plat->triangles[1].vector3[0]);

	for (float x = plat->triangles[1].vector2[0]; x <= min_x; x++) {
		float y1 = line_point(plat->triangles[1].vector2, plat->triangles[1].vector1, x, true);
		float z1 = line_point(plat->triangles[1].vector2, plat->triangles[1].vector1, x, false);

		float y2 = line_point(plat->triangles[1].vector2, plat->triangles[1].vector3, x, true);
		float z2 = line_point(plat->triangles[1].vector2, plat->triangles[1].vector3, x, false);

		float min_z = min(z1, z2);
		float max_z = max(z1, z2);

		if (max_z - min_z == 0) {
			if (y1 <= -3071) { continue; }
		}
		/*
		//(-3071 + min_y) / (max_y - min_y) * (max_z - min_z) + min_z;

		float check_min, check_max;

		if (min_z == z1) {
			check_min = max(min_z, (-3071 + y1) / (y2 - y1) * (max_z - min_z) + min_z);
			check_max = min(max_z, (-3071 + y2) / (y1 - y2) * (max_z - min_z) + min_z);
		}
		else {
			check_min = max(min_z, (-3071 + y2) / (y1 - y2) * (max_z - min_z) + min_z);
			check_max = min(max_z, (-3071 + y1) / (y2 - y1) * (max_z - min_z) + min_z);
		}

		float temp1 = min_z;
		float temp2 = max_z;

		if (check_min <= temp2) { min_z = check_min; }
		if (check_max >= temp1) { max_z = check_max; }*/

		for (float z = min_z; z <= max_z; z++) {
			float y;

			if (min_z == z1) {
				if (z2 - z1 == 0) {
					y = y1;
				}
				else {
					y = (y2 - y1) / (z2 - z1) * (z - z1) + y1;
				}
			}
			else {
				if (z1 - z2 == 0) {
					y = y2;
				}
				else {
					y = (y1 - y2) / (z1 - z2) * (z - z2) + y2;
				}
			}

			if (y <= -3071) { continue; }

			brute_angles(m, plat, { x, y, z }, spd, normals, trans);
			//fprintf(stderr, "finished all angles for position %.9f, %.9f, %.9f\n", x, y, z);

			//plat->transform = trans;

			for (int i = 0; i < 2; i++) { plat->triangles[i] = tri[i]; }
			for (int i = 0; i < 3; i++) { plat->normal[i] = normals[i]; }
		}
	}

	Vec3s max_vector;
	Vec3s min_vector;

	if (min_x == plat->triangles[1].vector1[0]) {
		for (int i = 0; i < 3; i++) { min_vector[i] = plat->triangles[1].vector1[i]; }
		for (int i = 0; i < 3; i++) { max_vector[i] = plat->triangles[1].vector3[i]; }
	}
	else {
		for (int i = 0; i < 3; i++) { min_vector[i] = plat->triangles[1].vector3[i]; }
		for (int i = 0; i < 3; i++) { max_vector[i] = plat->triangles[1].vector1[i]; }
	}

	for (float x = min_x; x <= max_x; x = x+1) {
		float y1 = line_point(plat->triangles[1].vector2, max_vector, x, true);
		float z1 = line_point(plat->triangles[1].vector2, max_vector, x, false);

		float y2 = line_point(min_vector, max_vector, x, true);
		float z2 = line_point(min_vector, max_vector, x, false);

		float min_z = min(z1, z2);
		float max_z = max(z1, z2);

		if (max_z - min_z == 0) {
			if (y1 <= -3071) { continue; }
		}

		for (float z = min_z; z <= max_z; z++) {
			float y;

			if (min_z == z1) {
				if (z2 - z1 == 0) {
					y = y1;
				}
				else {
					y = (y2 - y1) / (z2 - z1) * (z - z1) + y1;
				}
			}
			else {
				if (z1 - z2 == 0) {
					y = y2;
				}
				else {
					y = (y1 - y2) / (z1 - z2) * (z - z2) + y2;
				}
			}

			if (y <= -3071) { continue; }

			brute_angles(m, plat, { x, y, z }, spd, normals, trans);
			//fprintf(stderr, "finished all angles for position %.9f, %.9f, %.9f\n", x, y, z);

			//plat->transform = trans;
			for (int i = 0; i < 2; i++) { plat->triangles[i] = tri[i]; }
			for (int i = 0; i < 3; i++) { plat->normal[i] = normals[i]; }
		}
	}
}

void brute_normals(float spd) {

    float starting_normal =-1.0f;
    float ending_normal = 1.0f;
    float per_worker = (ending_normal - starting_normal) / omp_get_max_threads();
    vector<float> normals(omp_get_max_threads() + 1);
    for (int i = 0; i < omp_get_max_threads(); i++) {
        normals[i] = starting_normal + i * per_worker;
    }
    normals[omp_get_max_threads()] = ending_normal;

    OMP_FOR
    for (int i = 0; i < omp_get_max_threads(); i++) {
        Mario m;
        Platform p;
	    for (float nx = normals[i]; nx <= normals[i+1]; nx = nextafterf(nx, 2.0f)) {
		    float limit = powf(nx, 2) - 1.0f;

   		    for (float nz = limit; nz <= limit * -1; nz = nextafterf(nz, limit * -1 + 1)) {

			    float ny = sqrtf(1 - powf(nx, 2) - powf(nz, 2));

  			    brute_position(&m, &p, spd, {nx, ny, nz});

			    fprintf(stderr, "Finished all normals for %.9f, %.9f, %.9f\n", nx, ny,
				      	nz);
			}
		}
	}
}

void brute_speed() {
    float spd = 58000000.0f;
    float ending_spd = 1000000000.0;

    while (spd < ending_spd) {
        brute_normals(spd);

        spd = nextafterf(spd, 2000000000.0f);
        fprintf(stderr, "Finished all loops for speed %.9f\n", spd);
    }
}

int main() {
	brute_speed();
}