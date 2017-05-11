#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
using namespace std;

int main()
{
	ofstream fout("a.obj");
	fout<<"mtllib floor.mtl"<<endl;
	float x = -8;
	float y = -8;

	int n = 10;
	float offset = -2.0 * (x) / (n * 1.0);

	for(int i = 0; i < n + 1; i++)
		for(int j = 0; j < n + 1;j++)
		{
			fout<<"v "<<x + j * offset<<" "<<0.0<<" "<<y + i * offset<<endl;
		}

	fout<<endl;
	float vx = 0;
	float vy = 0;
	offset = 1.0 / (n * 1.0);

	for(int i = 0; i < n + 1; i++)
		for(int j = 0; j < n + 1;j++)
		{
			fout<<"vt "<<vx + j * offset<<" "<<vy + i * offset<<endl;
		}

	fout<<"vn 0 1 0"<<endl;
	fout<<"usemtl floor"<<endl;
	fout<<endl;

	for(int i = 1; i < n + 1; i++)
		for(int j = 1; j < n + 1; j++)
		{
			int lt = (i - 1) * (n + 1) + j;
			int rt = lt + 1;
			int lb = i * (n + 1) + j;
			int rb = lb + 1;
			fout<<"f "<<lt<<"/"<<lt<<"/"<<1<<" "<<rt<<"/"<<rt<<"/"<<1<<" "<<lb<<"/"<<lb<<"/"<<1<<endl;
			fout<<"f "<<rt<<"/"<<rt<<"/"<<1<<" "<<lb<<"/"<<lb<<"/"<<1<<" "<<rb<<"/"<<rb<<"/"<<1<<endl;
		}
	return 0;
}