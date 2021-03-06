//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "map.h"
#include "item.h"
#include "creature.h"
#include "globalvars.h"
#include "../objects.h"
#include "../notifications.h"
#include "../debugprint.h"

#include "../util.h"

extern uint32_t g_frameTime;

//*************** DistanceEffect **************************

DistanceEffect::DistanceEffect(const Position& from, const Position& to, uint32_t type) :
DistanceUI(from, to, type)
{
	//
}

//*************** Effect **************************

Effect::Effect(uint32_t type) :
EffectUI(type)
{
	//
}

//*************** AnimatedText **************************

AnimatedText::AnimatedText(const Position& pos, uint32_t color, const std::string& text)
{
	m_color = color;
	m_text = text;
	m_pos = pos;
	m_startTime = g_frameTime;
}

bool AnimatedText::canBeDeleted()
{
	if(g_frameTime - m_startTime > 1000){
		return true;
	}
	else{
		return false;
	}
}

//*************** PublicMessage *************************
PublicMessage::PublicMessage(TextColor_t color, const std::string& text, const std::string& sender, const Position& pos, bool showName, int linecount, MSG_RANGE range)
{
	m_color = color;
	m_sender = sender;
	m_text = text;
	m_pos = pos;
	m_startTime = g_frameTime;
    m_showName = showName;
	m_linecount = linecount;
    m_handled = false;

    m_relativePos = 0;
    m_range = range;
}

bool PublicMessage::canBeDeleted()
{
	if(g_frameTime - m_startTime > 3000){
		return true;
	}
	else{
		return false;
	}
}

void PublicMessage::set_handled(bool handled)
{
    if (handled == true)
        m_handled = true;
    else
        m_handled = false;
}

void PublicMessage::set_relativePos(int pos)
{
    m_relativePos = pos;
}

//*************** PrivateMessage *************************
PrivateMessage::PrivateMessage(TextColor_t color, const std::string& text, const std::string& sender, int linecount)
{
	m_color = color;
	m_sender = sender;
	m_text = text;
	//m_startTime = g_frameTime;
	m_linecount = linecount;
    //m_handled = false;
    m_onscreen = false;
}

void PrivateMessage::setOnScreen(bool screen)
{
    m_onscreen = screen;
    m_startTime = g_frameTime;
}

bool PrivateMessage::canBeDeleted()
{
	if(m_onscreen == true && g_frameTime - m_startTime > 2000 + (m_linecount * 1000))
		return true;
	else
		return false;
}

//*************** Tile **************************

Tile::Tile()
{
	m_ground = NULL;
}

Tile::~Tile()
{
	clear();
}

Thing* Tile::getThingByStackPos(int32_t pos)
{
	if(m_ground){
		if(pos == 0)
			return m_ground;

		pos--;
	}

	int size = m_objects.size();
	if(pos < size){
		return m_objects[pos];
	}

	return NULL;
}

const Thing* Tile::getThingByStackPos(int32_t pos) const
{
	if(m_ground){
		if(pos == 0)
			return m_ground;

		pos--;
	}

	int size = m_objects.size();
	if(pos < size){
		return m_objects[pos];
	}

	return NULL;
}

const Creature* Tile::getTopCreature() const
{
	for(int pos = 0; pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing) {
			return NULL;
		}

		const Creature* ret = thing->getCreature();
		if(ret) {
			return ret;
		}
	}

	return NULL;
}

int Tile::getUseStackpos() const
{
	int pos = 0;
	int lastPos = 0;
	for(;pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing) {
			return 0;
		}

		const Item* item = thing->getItem();

		if((item != NULL)){
			const Thing* lastThing = getThingByStackPos(lastPos);

			if(item->isAlwaysUsed()) {
				return pos;
			}
			if( (lastThing == NULL) || (thing->getOrder() > lastThing->getOrder()) ) {
				lastPos = pos;
			}
		}
	}

	return lastPos;
}

int Tile::getExtendedUseStackpos() const
{
	// TODO (nfries88): figure out the proper way to do this...
	int pos = 0;
	int lastPos = 0;
	int lastCreaturePos = 0;
	for(;pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing) {
			return 0;
		}

		const Item* item = thing->getItem();

		if((item != NULL)||(thing->getCreature() != NULL)){
			const Thing* lastThing = getThingByStackPos(lastPos);

			if(item && item->isAlwaysUsed()) {
				return pos;
			}
			else if( (lastThing == NULL) || (thing->getOrder() > lastThing->getOrder())) {
				lastPos = pos;
				if(thing->getCreature() != NULL)
				{
					lastCreaturePos = pos;
				}
			}
		}
	}
	if(lastCreaturePos == 0)
		return lastPos;
	return lastPos;
}

bool Tile::isTileBlocking() const
{
	int pos = 0;
	int lastPos = 0;
	int lastCreaturePos = 0;
	for(; pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing) {
			return 0;
		}

		const Item* item = thing->getItem();
		const Creature* creature = thing->getCreature();

		if((item != NULL)||(creature != NULL)) {
			const Thing* lastThing = getThingByStackPos(lastPos);

			if(item && item->isBlocking()) {
				return true;
			}

			if(creature && creature->isImpassable()) {
				return true;
			}

		}
	}

	return false;
}

bool Tile::isWall() const
{
	int pos = 0;
	for(; pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing)
			return false;

		const Item* item = thing->getItem();

		if(item)
			if(pos == 1 && (item->getObjectType()->isHorizontal || item->getObjectType()->isVertical))
				return true;
	}

	return false;
}

bool Tile::canSeeThrough() const
{
    int pos = 0;
	for(; pos != getThingCount(); ++pos) {
		const Thing* thing = getThingByStackPos(pos);
		if(!thing)
			return false;

		const Item* item = thing->getItem();

		if(item)
			if(item->getObjectType()->blockProjectile)
				return false;
	}

	return true;
}

const Item* Tile::getGround() const
{
	return m_ground;
}

void Tile::clear()
{
	delete m_ground;
	m_ground = NULL;

	for(ThingVector::iterator it = m_objects.begin(); it != m_objects.end(); ++it){
		Creature* c = (*it)->getCreature();
		if(c != NULL)
		{
			c->setCurrentPos(Position(0, 0, 0));
			Notifications::onCreatureMove(c->getID(), getPos(), c->getCurrentPos());
		}
		Thing::deleteThing(*it);
	}

	m_objects.clear();
	m_effects.clear();
}

int32_t Tile::getThingCount() const
{
	int32_t objectCount = m_objects.size();
	return (m_ground ? 1 : 0) + objectCount;
}

bool Tile::insertThing(Thing *thing, int32_t stackpos)
{
	if(thing == NULL){
		return false;
	}

	if(getThingCount() == 10){
		Thing* pushThing = getThingByStackPos(9);
		if(!removeThing(pushThing)){
			return false;
		}
		Thing::deleteThing(pushThing);
	}

	int pos = stackpos;

	Item* item = thing->getItem();
	if(item != NULL){
		if(item->isGroundTile() && pos == 0){
			if(m_ground){
				Thing::deleteThing(m_ground);
			}
			m_ground = item;
			return true;
		}

		if(item->isGroundTile() && pos != 0){
			return false;
		}
	}

	if(pos != 255){
		if(m_ground){
			--pos;
		}

		ThingVector::iterator it = m_objects.begin();
		while(pos > 0 && it != m_objects.end()){
			pos--;
			++it;
		}
		if(pos > 0){
			return false;
		}

		m_objects.insert(it, thing);
	}
	else {
		addThing(thing, true);
	}
	Creature *c = thing->getCreature();
	if(c){
		c->setCurrentPos(this->getPos());
	}

	return true;
}

bool Tile::removeThing(Thing *thing)
{
	if(thing == NULL){
		return false;
	}

	if(thing == m_ground){
		m_ground = NULL;
		return true;
	}

	ThingVector::iterator it = std::find(m_objects.begin(), m_objects.end(), thing);
	if(it != m_objects.end()){
		m_objects.erase(it);
		return true;
	}

	return false;
}

bool Tile::addThing(Thing *thing, bool pushThing/* = false*/)
{
	if(thing == NULL){
		return false;
	}

	Item* item = thing->getItem();

	if(item && item->isGroundTile()){  //yes, I know this could potentially fsck up if the tile already has 10 non-ground things on it and ground is added to that, but if a server does that it does not deserve to be graced by YATC's presence.
		if(m_ground != NULL){
			return false;
		}

		m_ground = item;
		return true;
	}

	int thingOrder = thing->getOrder();
	ThingVector::iterator it;

    // TODO (nfries88): thomac says that for creatures, version 8.54+, must be added at the BOTTOM of their order; not the top.
	for(it = m_objects.begin(); it != m_objects.end(); ++it){
		int itThingOrder = (*it)->getOrder();
		if(pushThing){
			if(itThingOrder >= thingOrder){
				break;
			}
		}
		else{
			if(itThingOrder > thingOrder){
				break;
			}
		}
	}

	m_objects.insert(it, thing);

	if(getThingCount() > 10){
		Thing* excessThing = getThingByStackPos(10);
		if(!removeThing(excessThing)){
			return false;
		}
		Thing::deleteThing(excessThing);
	}

	Creature *c = thing->getCreature();
	if(c){
		c->setCurrentPos(this->getPos());
	}
	return true;
}

void Tile::addEffect(uint32_t effect)
{
	m_effects.push_back(new Effect(effect));
}

uint16_t Tile::getSpeedIndex() const
{
    return getGround() ? Objects::getInstance()->getItemType( getGround()->getID() )->speed : 500;
}
uint8_t Tile::getMinimapColor() const
{
    ThingVector::const_iterator it;
    uint8_t color=0;
    if(getGround()) color = getGround()->getMapColor();
    for(it = m_objects.begin(); it != m_objects.end(); ++it){
        if((*it)->getMapColor()) color=(*it)->getMapColor();
    }
    return color;
}

bool Tile::blockPath() const
{
	if(m_ground){
		const ObjectType* obj = m_ground->getObjectType();
		if(obj->blockPathFind) return true;
	}
	for(ThingVector::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it){
		if((*it)->getCreature()) return true;

		Item* item = (*it)->getItem();
		if(item != NULL){
			const ObjectType* obj = item->getObjectType();
			if(obj->blockPathFind) return true;
		}
	}
	return false;
}


//*************** Map **************************

#define POS2INDEX(x, y, z) ((uint64_t)(x & 0xFFFF) << 24 | (y & 0xFFFF) << 8 | (z & 0xFF))
#define INDEX2POS(index, pos)  pos.x = ((uint64_t)(index) >> 24) & 0xFFFF; pos.y = ((index) >> 8) & 0xFFFF; pos.z = ((index) & 0xFF);

Map::Map()
{
	m_freeTiles.reserve(TILES_CACHE);
	for(uint32_t i = 0; i < TILES_CACHE; ++i){
		m_freeTiles.push_back(i);
	}
}

Map::~Map()
{
	clear();
}

void Map::clear()
{
	for(uint32_t i = 0; i < TILES_CACHE; ++i){
		m_tiles[i].clear();
	}
	m_coordinates.clear();
	m_freeTiles.clear();
	m_freeTiles.reserve(TILES_CACHE);
	for(uint32_t i = 0; i < TILES_CACHE; ++i){
		m_freeTiles.push_back(i);
	}
}

Tile* Map::setTile(uint32_t x, uint32_t y, uint32_t z)
{
	Tile* tile = getTile(x, y, z);
	if(!tile){
		if(m_freeTiles.size() == 0){
			CoordMap::iterator it;
			uint32_t freedTiles = 0;
			for(it = m_coordinates.begin(); it != m_coordinates.end(); ){
				Position pos;
				INDEX2POS(it->first, pos);
				if(!playerCanSee(pos.x, pos.y, pos.z)){

					m_freeTiles.push_back(it->second);
					m_coordinates.erase(it++);

					++freedTiles;
					if(freedTiles > 384){
						break;
					}
				}
				else{
					++it;
				}
			}

			if(freedTiles == 0){
				//No free tiles after looking for them
				return NULL;
			}
		}

		uint32_t i = m_freeTiles.back();
		m_freeTiles.pop_back();

		uint64_t posIndex = POS2INDEX(x, y, z);
		m_coordinates[posIndex] = i;

		m_tiles[i].setPos(Position(x,y,z));

		return &m_tiles[i];
		return NULL;
	}
	else{
		return tile;
	}
}

Tile* Map::getTile(uint32_t x, uint32_t y, uint32_t z)
{
	uint64_t posIndex = POS2INDEX(x, y, z);
	CoordMap::iterator it = m_coordinates.find(posIndex);
	if(it != m_coordinates.end()){
		return &m_tiles[it->second];
	}
	else{
		return NULL;
	}
}

const Tile* Map::getTile(uint32_t x, uint32_t y, uint32_t z) const
{
	uint64_t posIndex = POS2INDEX(x, y, z);
	CoordMap::const_iterator it = m_coordinates.find(posIndex);
	if(it != m_coordinates.end()){
		return &m_tiles[it->second];
	}
	else{
		return NULL;
	}
}

bool Map::playerCanSee(int32_t x, int32_t y, int32_t z)
{
	const Position& playerPos = GlobalVariables::getPlayerPosition();
	if(playerPos.z <= 7){
		//we are on ground level or above (7 -> 0)
		//view is from 7 -> 0
		if(z > 7){
			return false;
		}
	}
	else if(playerPos.z >= 8){
		//we are underground (8 -> 15)
		//view is +/- 2 from the floor we stand on
		if(abs((int32_t)playerPos.z - z) > 2){
			return false;
		}
	}

	//negative offset means that the action taken place is on a lower floor than ourself
	int offsetz = playerPos.z - z;

	if((x >= (int32_t)playerPos.x - 9 + offsetz) && (x <= (int32_t)playerPos.x + 10 + offsetz) &&
		(y >= (int32_t)playerPos.y - 7 + offsetz) && (y <= (int32_t)playerPos.y + 8 + offsetz)){
		return true;
	}

	return false;
}

void Map::addDistanceEffect(const Position& from, const Position& to, uint32_t type)
{
	ASSERT(from.z == to.z);
	ASSERT(from.z < MAP_LAYER);
	m_distanceEffects[from.z].push_back(new DistanceEffect(from, to , type));
}

void Map::addAnimatedText(const Position& pos, uint32_t color, const std::string& text)
{
	ASSERT(pos.z < MAP_LAYER);
	m_animatedTexts[pos.z].push_back(AnimatedText(pos, color, text));
}

void Map::addPublicMessage(const Position& pos, TextColor_t color, const std::string& text, const std::string& sender, MSG_RANGE range, bool showName)
{
	ASSERT(pos.z < MAP_LAYER);

	std::string new_line_text, old_line_text;
    std::stringstream final_text;

    new_line_text = old_line_text = text;
    int linecount = 1;
    int line_size = text.length();
    size_t iter_pos;

    // NOTE (Kilouco): Here we make linebreaks for public messages.
    // TODO (Kilouco): Generic Linebreaker.
    //39 characters per line, despite their size.
    if (line_size > 39) {
        while (line_size > 39) {
            old_line_text.erase(old_line_text.end()-1);
            line_size = old_line_text.length();

            if (line_size <= 39) {
                iter_pos = old_line_text.find_last_of(" ");

                if(iter_pos == std::string::npos || !iter_pos) {
                    iter_pos = old_line_text.length();
                    new_line_text = new_line_text.substr(iter_pos-1);
                    old_line_text.resize(iter_pos);

                    old_line_text.erase(old_line_text.end()-1);
                    old_line_text = old_line_text + "-";
                    iter_pos = old_line_text.length();
                }
                else {
                    new_line_text = new_line_text.substr(iter_pos);
                    old_line_text.resize(iter_pos);
                }

                final_text << old_line_text << "\n";

                linecount++;

                old_line_text = new_line_text;
                line_size = old_line_text.length();

                if (line_size <= 39)
                    final_text << new_line_text;
            }
        }
        m_publicMessages.push_back(PublicMessage(color, final_text.str(), sender, pos, showName, linecount, range));
        return;
    }
	m_publicMessages.push_back(PublicMessage(color, text, sender, pos, showName, linecount, range));
}

void Map::addPrivateMessage(TextColor_t color, const std::string& text, const std::string& sender)
{
	std::string new_line_text, old_line_text;
    std::stringstream final_text;

    new_line_text = old_line_text = text;
    int linecount = 1;
    int line_size = text.length();
    size_t iter_pos;

    // NOTE (Kilouco): Here we make linebreaks for public messages.
    // TODO (Kilouco): Generic Linebreaker.
    //39 characters per line, despite their size.
    if (line_size > 39) {
        while (line_size > 39) {
            old_line_text.erase(old_line_text.end()-1);
            line_size = old_line_text.length();

            if (line_size <= 39) {
                iter_pos = old_line_text.find_last_of(" ");

                if(iter_pos == std::string::npos || !iter_pos) {
                    iter_pos = old_line_text.length();
                    new_line_text = new_line_text.substr(iter_pos-1);
                    old_line_text.resize(iter_pos);

                    old_line_text.erase(old_line_text.end()-1);
                    old_line_text = old_line_text + "-";
                    iter_pos = old_line_text.length();
                }
                else {
                    new_line_text = new_line_text.substr(iter_pos);
                    old_line_text.resize(iter_pos);
                }

                final_text << old_line_text << "\n";

                linecount++;

                old_line_text = new_line_text;
                line_size = old_line_text.length();

                if (line_size <= 39)
                    final_text << new_line_text;
            }
        }
        m_privateMessages.push_back(PrivateMessage(color, final_text.str(), sender, linecount));
        return;
    }
    m_privateMessages.push_back(PrivateMessage(color, text, sender, linecount));
}

std::list<Direction> Map::getPathTo(int x, int y, int z)
{
	// keep it on the map
	#define MAX_SEARCH_DIST 25
	std::list<Direction> path;
	path.clear();
	// make sure we can walk to the tile to begin with.
	Tile* ttile = getTile(x, y, z);
	if(!ttile || ttile->blockPath()) return path;

	// TODO (nfries88): Actually calculate the path.

	return path;
}
