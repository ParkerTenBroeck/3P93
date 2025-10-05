#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vec_math.h"
#include "types.h"

int main(){
	Matrix3<f32> m{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
	};

	auto view = Matrix4<f32>::look_at(
		{0,0,0},
		{1,1,1},
		{0,1,0}
		);
	view.print();
}
