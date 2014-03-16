#include "Map.h"

Map::Map() : roomX(1), roomY(1){
	room_exit_x = 0;
	room_exit_y = roomX/2*ysize;
	room_entry_x = 0;
	room_entry_y = roomX / 2 * ysize;
}

double Map::alterBeingPosX(double absoluteX){
	absoluteX += offsetx;
	if (absoluteX < 0.5)return absoluteX;
	if (absoluteX > roomX - 0.5)return absoluteX - roomX + 1;
	return 0.5;
}

double Map::alterBeingPosY(double absoluteY){
	absoluteY += offsety;
	if (absoluteY < 0.5)return absoluteY;
	if (absoluteY > roomY - 0.5)return absoluteY - roomY + 1;
	return 0.5;
}

void Map::loadMap(string seed){
	seed_seq Seed(seed.begin(), seed.end());
	uint32_t seeds[1];
	Seed.generate(&seeds[0], &seeds[1]);
	curr_seed = seeds[0];
	curr_seed = system_clock::to_time_t(system_clock::now()); //SHOULD BE FIXED 
	generateRoom(curr_seed, true);
}

Map::Coord Map::getMapEntry(){
	if (!last_entry){
		offsetx = (double)(room_entry_x-room_exit_x) / xsize;
		offsety = (double)(room_entry_y-room_exit_y) / ysize;
		return Coord((double)(room_entry_x) / xsize, (double)(room_entry_y) / ysize, 0);
	}
	offsetx = (double)(room_exit_x - room_entry_x) / xsize;
	offsety = (double)(room_exit_y - room_entry_y) / ysize;
	return Coord((double)(room_exit_x) / xsize, (double)(room_exit_y) / ysize, 0);
}

bool Map::updateInternalMapState(){
	bool needs_time = false;
	if (offsetx > 0.01){ offsetx -= 0.01; needs_time = true; }
	if (offsety > 0.01){ offsety -= 0.01; needs_time = true; }
	if (offsetx < -0.01){ offsetx += 0.01; needs_time = true; }
	if (offsety < -0.01){ offsety += 0.01; needs_time = true; }
	return needs_time;
}

bool Map::tryRoomChange(int x, int y){
	bool exit = false;
	bool isexit;
	if (x < 0 || x >= room.size()){
		isexit = abs(room_entry_x-x) > abs(room_exit_x-x) ? true : false;
		exit = true;
	}
	if (y < 0 || y >= room[0].size()){		//SHOULD BE FIXED
		isexit = abs(room_entry_y - y) > abs(room_exit_y - y) ? true : false;
		exit = true;
	}
	if (exit){
		if (isexit){
			generateRoom(++curr_seed, 1);
			last_entry = false;
		}
		if (!isexit){
			generateRoom(--curr_seed, 0);
			last_entry = true;
		}
	}
	return exit;

}

int Map::getMapType(){
	return map_type;
}

void Map::generateRoom(uint32_t seed_, bool exited){
	//int num_tiles = 16;//hardcode!
	//int coef = (2*roomx*roomy);
	//int num_structures = coef + pattern() % coef;
	//int yplaces = roomy * num_tiles;	
	//int xplaces = roomx * num_tiles;	
	//vector<coord> blocks;
	//int direction;
	//int x, y;
	//int num_blocks;
	//for (int i = 0; i < num_structures; ++i){
	//	//int structure_type = pattern() % 6;
	//	int structure_type = pattern()%6; ///!!!
	//	switch (structure_type){
	//	case 0://straight line from wall
	//		num_blocks = pattern() % num_tiles-1;
	//		direction =pattern() % 4;
	//		switch (direction){
	//		case 0://up
	//			x = pattern() % xplaces;
	//			for (int a = 0; a < num_blocks; ++a){
	//				coord c;
	//				c.x = (double)x / num_tiles;
	//				c.y = roomy - 1 / num_tiles - (double)a / num_tiles;
	//				c.partx = 1;
	//				c.party = 1;
	//				blocks.push_back(coord((double)x / num_tiles, roomy - 1 / num_tiles - (double)a / num_tiles));
	//			}
	//			break;
	//		case 1://down
	//			x = pattern() % xplaces;
	//			for (int a = 0; a < 1; ++a){
	//				blocks.push_back(coord((double)x / num_tiles, (double)a / num_tiles));
	//			}
	//			break;
	//		case 2://left
	//			y = pattern() % yplaces;
	//			for (int a = 0; a < num_blocks; ++a){
	//				blocks.push_back(coord((double)a / num_tiles, (double)y / num_tiles));
	//			}
	//			break;
	//		case 3://right
	//			y = pattern() % yplaces;
	//			for (int a = 0; a < num_blocks; ++a){
	//				blocks.push_back(coord(roomx - 1 / num_tiles - (double)a / num_tiles, (double)y / num_tiles));
	//			}
	//			break;
	//		}
	//		break;
	//	case 1://three-line zig-zag from wall
	//		//num_blocks = pattern() % (num_tiles*3 - 10);
	//		break;
	//	case 2://two-line zig-zag from wall
	//		break;
	//	case 3://straight line
	//		break;
	//	case 4://three-line zig-zag
	//		break;
	//	case 5://three-line perpendicular
	//		break;
	//	}
	//}
	//for (int i = 0; i < xplaces; ++i){
	//	blocks.push_back(coord(0, (double)i / num_tiles));
	//	blocks.push_back(coord(roomx - roomx / xplaces, (double)i / num_tiles));
	//}
	//for (int i = 0; i < yplaces; ++i){
	//	blocks.push_back(coord((double)i / num_tiles, 0));
	//	blocks.push_back(coord((double)i / num_tiles, roomy - roomy / xplaces));
	//}
	pattern.seed(seed_);
	uint32_t cx = pattern() % 2000;
	uint32_t cy = pattern() % 2000;
	uint32_t r1 = pattern();
	uint32_t r2 = 1 + (pattern() % 3);
	uint32_t r3 = pattern();
	int last_exit_x = room_exit_x;
	int last_exit_y = room_exit_y;
	int last_entry_x = room_entry_x;
	int last_entry_y = room_entry_y;
	int xmax = xsize*roomX - 1;
	int ymax = ysize*roomY - 1;

	int curr_wall, curr_x, curr_y;
	if (exited){
		curr_x = last_exit_x;
		curr_y = last_exit_y;
	}
	else{
		curr_x = last_entry_x;
		curr_y = last_entry_y;
	}

	if (curr_x == 0)curr_wall = LEFT;
	if (curr_x == xmax)curr_wall = RIGHT;
	if (curr_y == 0)curr_wall = UP;
	if (curr_y == ymax)curr_wall = DOWN;


	roomX = 1 + (double)cx/1000.0;
	roomY = 1 + (double)cy/1000.0;

	xmax = xsize*roomX - 1;
	ymax = ysize*roomY - 1;

	int temp = min(ymax, xmax) - 4;
	int newplace = 2;

	if (exited)newplace += r1 % temp;
	else newplace += r3 % temp;

	switch(curr_wall){
		case LEFT:
			curr_x = xmax;
			curr_y = newplace;
			break;
		case RIGHT:
			curr_x = 0;
			curr_y = newplace;
			break;
		case UP:
			curr_y = ymax;
			curr_x = newplace;
			break;
		case DOWN:
			curr_y = 0;
			curr_x = newplace;
			break;
	}

	if (exited){
		room_entry_x = curr_x;
		room_entry_y = curr_y;
	}
	else{
		room_exit_x = curr_x;
		room_exit_y = curr_y;
	}

	newplace = 2;

	int opposite_wall = 3 - curr_wall;

	if (exited){
		opposite_wall += r2;
		opposite_wall %= 4;
	}
	else{
		opposite_wall += (4-r2);
		opposite_wall %= 4;
	}
	newplace = 2;
	if (exited)newplace += r3 % temp;
	else newplace += r1 % temp;

	switch (opposite_wall){
	case UP:
		curr_y = 0;
		curr_x = newplace;
		break;
	case DOWN:
		curr_y = (ysize*roomY - 1);
		curr_x = newplace;
		break;
	case RIGHT:
		curr_x = (xsize*roomX - 1);
		curr_y = newplace;
		break;
	case LEFT:
		curr_x = 0;
		curr_y = newplace;
		break;
	}

	if (exited){
		room_exit_x = curr_x;
		room_exit_y = curr_y;
	}
	else{
		room_entry_x = curr_x;
		room_entry_y = curr_y;
	}
	room = *new vector<vector<char>>(roomX*xsize, vector<char>(roomY*ysize, 0));
	int xplaces = roomX*xsize, yplaces = roomY*ysize;
	for (int i = 0; i < xplaces; ++i){
		room[i][0] = 1;
		room[i][yplaces - 1] = 1;
		if (i == 0){
			room[i][0] = 2;
			room[i][yplaces - 1] = 2;
		}
	}
	for (int i = 0; i < yplaces; ++i){
		room[0][i] = 3;
		room[xplaces - 1][i] = 3;
		if (i == 0){
			room[0][i] = 2;
			room[xplaces - 1][i] = 1;
		}
		if (i == yplaces - 1){
			room[xplaces - 1][i] = 4;
		}
	}
	room[room_entry_x][room_entry_y] = 0;
	room[room_exit_x][room_exit_y] = 0;

	//generate some basic structures
	int numstructs = pattern() % 20;
	numstructs *= roomX*roomY;
	for (int i = 0; i < numstructs; ++i){
		int structsize = 2 + (pattern() % 5);
		int structori = pattern() % 2;//X:Y
		int structdir = pattern() % 2;
		int initialx = pattern() % xmax;
		int initialy = pattern() % ymax;

		int endx, endy;
		bool begin;
		switch (structori){
		case 0: //orientation X
			endx = initialx + structsize;
			endy = initialy;
			if (!structdir)endx = initialx + structsize;
			else endx = initialx - structsize;
			begin = true;
			for (int a = min(initialx, endx); a < max(initialx, endx); ++a){
				int x = a;
				int y = endy;
				try{
					if (!room.at(x-1).at(y-1)){
						if (!room.at(x).at(y - 1)){
							if (!room.at(x + 1).at(y - 1)){
								if (!room.at(x + 1).at(y)){
									if (!room.at(x + 1).at(y + 1)){
										if (!room.at(x).at(y + 1)){
											if (!room.at(x - 1).at(y + 1)){
												if (!room.at(x - 1).at(y) || begin == false){
													room[x][y] = 1;
												}
											}
										}
									}

								}
							}
						}
					}
				}
				catch (...){}
				begin = false;
			}
			break;
		case 1: //orientation Y
			endx = initialx;
			if (!structdir)endy = initialy + structsize;
			else endy = initialy - structsize;
			begin = true;
			for (int a = min(initialy, endy); a < max(initialy, endy); ++a){
				int x = endx;
				int y = a;
				try{
					if (!room.at(x - 1).at(y - 1)){
						if (!room.at(x).at(y - 1) || begin == false){
							if (!room.at(x + 1).at(y - 1)){
								if (!room.at(x + 1).at(y)){
									if (!room.at(x + 1).at(y + 1)){
										if (!room.at(x).at(y + 1)){
											if (!room.at(x - 1).at(y + 1)){
												if (!room.at(x - 1).at(y)){
													room[x][y] = 3;
												}
											}
										}
									}

								}
							}
						}
					}
				}
				catch (...){}
				begin = false;
			}
			break;
		}
	}
	map_type = pattern() % 3;
	//generate some basic structures

	//create map map
	blocks.clear();
	for (int x = 0; x < room.size(); ++x){
		for (int y = 0; y < room[x].size(); ++y){
			if (room[x][y])blocks.push_back(Coord((double)x/xsize, (double)y/ysize, room[x][y]));
		}
	}
}

const vector<Map::Coord>& Map::getMapObjects() const{
	return blocks;
}

const vector<vector<char>>& Map::getMapIndex() const{
	return room;
}