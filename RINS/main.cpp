#include "Platform.h"
#include "Being.h"
#include "Map.h"
#include <math.h>
#include <mutex>
class RINS : public Game, public Map{
	Renderer rend;

	int bg[3];
	int wall[3];
	int side[3][2];
	int entrytex, exittex, red;

	int dir, tmpdir2;
	double lastxpos, lastypos, deltax, deltay;
	Coord c;
	Being* player;

	list<unique_ptr<Being>> monsters;
	mt19937 pattern;

	int last_tick = 0, timer2 = 0, projectile_tick = 0;

	mutex lock1, monster, projectile;

	int highscore = 0, spawned = 0, lastroom = 0;
	int main_font;
	bool completed = false;

	void graphicsLoop() final {
		try{

			lock1.lock();
			renderMap();
			lock1.unlock();

			int offset = 0;
			if (player->getWalk())offset = 2;
			rend.renderPart(4, 2, offset+((int)log2(player->getOrientation()) >> 1), 1 - ((int)log2(player->getOrientation()) % 2));

			rend.applyTexture(BeingResources::getTextureID(typeid(*player).name()), alterBeingPosX(player->getX()), alterBeingPosY(player->getY()), 1.0 / xsize, 1.0 / ysize);

			monster.lock();//lock the monster!
			for (auto &i : monsters) {
				if (i->getWalk())rend.renderPart(4, 2, 2+((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				else rend.renderPart(4, 2, ((int)log2(i->getOrientation()) >> 1), 1 - ((int)log2(i->getOrientation()) % 2));
				rend.applyTexture(BeingResources::getTextureID(typeid(*i).name()), i->getX()-deltax, i->getY()-deltay, 1.0 / xsize, 1.0 / ysize);
			}
			monster.unlock();

			projectile.lock();
			for (auto &i : Being::projectiles){
				rend.applyTexture(WeaponResources::getTexture(i.getType()), i.getX() - deltax + 1.5*player->getStepX(), i.getY() - deltay + 1.5*player->getStepY(), player->getStepX(), player->getStepY());
			}
			projectile.unlock();

			rend.renderScene();
		}
		catch (Error e){
			cout << e.getError() << endl;
		}
	}

	void mainLoop() final {
		try {

			tmpdir2 = dir;
			getdir();

			lastxpos = player->getX();
			lastypos = player->getY();

			if (updateInternalMapState()) dir = 0;

			if (getTicks() - last_tick > 33){
				player->move(dir, false);
				last_tick = getTicks();
				if(lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
			}

			if (getTicks() - projectile_tick > 20){
				if (dir & 16){
					projectile.lock();
					player->shootWeapon();
					projectile.unlock();
				}
				projectile_tick = getTicks();
			}

			for (auto p = begin(Being::projectiles); p != end(Being::projectiles); ++p){
				bool res = p->update(getMapIndex(), monsters);
				if (!res){
					projectile.lock();
					p = Being::projectiles.erase(p);
					projectile.unlock();
				}
			}

			int x_colide, y_colide;
			int event = player->checkCollisions(lastxpos, lastypos, getMapIndex(), x_colide, y_colide);
			switch (event){
			case OUT_OF_BOUNDS:
				if (!completed)break;
				lock1.lock();
				if (tryRoomChange(x_colide, y_colide)){
					c = getMapEntry();
					player->setX(c.x);
					player->setY(c.y);
					if (getLastExploredRoom() > lastroom){
						completed = false;
						spawned = 0;
					}
				}
				lock1.unlock();
				break;
			case X_COLIDE:
			case Y_COLIDE:
			case XY_COLIDE:
				if(lastxpos == player->getX() && lastypos == player->getY())player->resetWalk();
				break;
			}

			bool mustspawn = pattern()%getSpawnRate()==false;
			int spawntype = pattern()%Being::monsters.size();
			int x = pattern()%getMapIndex().size();
			int y = pattern()%getMapIndex()[x].size();
			if(mustspawn && !getMapIndex()[x][y]){
				if (spawned == getMaxMonsters()){
					if (monsters.size() == 0){
						completed = true;
						lastroom = getLastExploredRoom();
					}
				}
				else{
					monster.lock();
					monsters.push_back(unique_ptr<Being>(Being::monsters[spawntype]((double)x / xsize, (double)y / ysize)));
					++spawned;
					monster.unlock();
				}
			}
			
			for (auto m = begin(monsters); m != end(monsters); ++m){
				bool res = (*m)->action(getMapIndex());
				if (!res){
					monster.lock();
					m = monsters.erase(m);
					monster.unlock();
				}
			}

			SDL_Delay(10);
		}
		catch (Error e) {
			cout << e.getError() << endl;
		}
	}

	void getdir(){
		if (getKey(1) == 'a')dir |= 1 << 0;
		if (getKey(0) == 'a')dir &= ~(1 << 0);
		if (getKey(1) == 'd')dir |= 1 << 1;
		if (getKey(0) == 'd')dir &= ~(1 << 1);
		if (getKey(1) == 'w')dir |= 1 << 2;
		if (getKey(0) == 'w')dir &= ~(1 << 2);
		if (getKey(1) == 's')dir |= 1 << 3;
		if (getKey(0) == 's')dir &= ~(1 << 3);
		if (getKey(1) == ' ')dir |= 1 << 4;
		if (getKey(0) == ' ')dir &= ~(1 << 4);
	}

	Uint16* itow(unsigned int h){
		int a = log10(h) + 1;
		if(a < 0) a = 1;
		Uint16* text = new Uint16[a + 1];
		for (int i = a - 1; i >= 0; --i){
			text[i] = (h % 10) + 48;
			h /= 10;
		}
		if(text[0] == 0) text[0] = '0';
		text[a] = 0;
		return text;
	}
	void renderMap(){
		int maptype = getMapType();
		deltax = player->getX() - alterBeingPosX(player->getX());
		deltay = player->getY() - alterBeingPosY(player->getY());
		rend.renderPart(0, 0, 0, 0);
		rend.applyTexture(bg[maptype], - deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
		rend.renderPart(0, 0, 0, 0);
		if(completed)rend.applyTexture(red, -deltax, -deltay, (double)(getMapIndex().size() / (double)xsize), (double)(getMapIndex()[0].size() / (double)ysize));
		double room_x, room_y;
		char wpos, hpos;
		getRoomSize(room_x, room_y);
		for (int i = 0; i < getMapObjects().size(); ++i){
			double block_x = getMapObjects().at(i).x;
			double block_y = getMapObjects().at(i).y;
			if (block_x == 0)wpos = 0;
			else if (block_x == room_x-1.0/xsize)wpos = 1;
			else wpos = 2;
			if (block_y == 0)hpos = 0;
			else if (block_y == room_y-1.0/ysize)hpos = 1;
			else hpos = 2;
			double x = block_x - deltax;
			double y = block_y - deltay;
			switch (getMapObjects().at(i).type){
			case 1:
				rend.applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 2:
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 3:
				rend.applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case 4:
				rend.applyTexture(wall[maptype], x, y - 1.0 / (2 * ysize), 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x - 1.0 / (2 * xsize), y, 1.0 / xsize, 1.0 / ysize);
				rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			case ENTRY:
				if (wpos == 0)rend.setRotationAngle(0);
				if (hpos == 0)rend.setRotationAngle(90);
				if (wpos == 1)rend.setRotationAngle(180);
				if (hpos == 1)rend.setRotationAngle(270);
				rend.applyTexture(entrytex, x, y, 1.0 / xsize, 1.0 / ysize);
				rend.setRotationAngle(0);
				break;
			case EXIT:
				if (wpos == 1)rend.setRotationAngle(0);
				if (hpos == 1)rend.setRotationAngle(90);
				if (wpos == 0)rend.setRotationAngle(180);
				if (hpos == 0)rend.setRotationAngle(270);
				rend.applyTexture(exittex, x, y, 1.0 / xsize, 1.0 / ysize);
				rend.setRotationAngle(0);
				break;
			default:
				//rend.applyTexture(wall[maptype], x, y, 1.0 / xsize, 1.0 / ysize);
				break;
			}
		}
		rend.applyTexture(side[maptype][0], -1, 0, 1, 1);
		rend.applyTexture(side[maptype][1], 1 , 0, 1, 1);
	}
public:
	RINS() try : 
		rend(640, 640, "RINS"), dir(0), c(0, 0, 0) {
		loadMap("do u even seed, bro?");
		c = getMapEntry();

		Being::setNumTiles(xsize, ysize);

		int pclass;
		cout << "Chose class: " << endl << "0. Marine " << endl << "1. Pyro " << endl << "2. Psychokinetic" << endl << "3. Android" << endl;
		//cin >> pclass;
		pclass = pclass%3;
		if(pclass == 0) player = new Marine(c.x,c.y);
		else if(pclass == 1) player = new Pyro(c.x, c.y);
		else if(pclass == 2) player = new Psychokinetic(c.x, c.y);
		else if(pclass == 3) player = new Android(c.x, c.y);


		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Marine).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Pyro).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Psychokinetic).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/devil2.png"), typeid(Android).name());
		BeingResources::addTextureID(rend.loadTexture("Textures/gangsta2.png"), typeid(Zombie).name());

		bg[0] = rend.loadTexture("Textures/floor1.jpg");
		bg[1] = rend.loadTexture("Textures/cement.jpg");
		bg[2] = rend.loadTexture("Textures/dirt2.jpg");

		wall[0]  = rend.loadTexture("Textures/brick3.png");
		wall[1]  = rend.loadTexture("Textures/brick4.png");
		wall[2]  = rend.loadTexture("Textures/brick5.png");

		side[0][0] = rend.loadTexture("Textures/school_1.png");
		side[0][1] = rend.loadTexture("Textures/school_2.png");
		side[1][0] = rend.loadTexture("Textures/hospital_1.png");
		side[1][1] = rend.loadTexture("Textures/hospital_2.png");
		side[2][0] = rend.loadTexture("Textures/forest_1.png");
		side[2][1] = rend.loadTexture("Textures/forest_2.png");

		WeaponResources::addTexture(rend.loadTexture("Textures/bullet.png"), BULLET);
		WeaponResources::addTexture(rend.loadTexture("Textures/bullet2.png"), FIRE);

		entrytex = rend.loadTexture("Textures/entry.png");
		exittex = rend.loadTexture("Textures/exit.png");
		red = rend.loadTexture("Textures/red.png");
		rend.setModulateBlending(red);

		main_font = rend.loadFont("Fonts/ARIALUNI.TTF", 40);

		Being::monsters[ZOMBIE] = &createInstance<Zombie>;

		Being::targets.push_back(player);

		//::xsize = xsize;
		//::ysize = ysize;

		loop();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}

	~RINS() {
		delete player;
	}
};

int main(int argc, char** argv) {
	try{
		RINS rins;
	}
	catch (...){
		system("pause");
	}
	return 0;
}