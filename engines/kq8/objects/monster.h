/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KQ8_OBJECTS_MONSTER_H
#define KQ8_OBJECTS_MONSTER_H

#include "audio/mixer.h"
#include "common/hash-ptr.h"
#include "common/hashmap.h"

#include "kq8/objects/anim_object.h"
#include "kq8/objects/object.h"
#include "kq8/singletons/inventory_item_type_list.h"

namespace Kq8 {

class Monster : public AnimObject {
	struct SpeakingState {
		uint16 _catalog;
		uint8 _talker;
		uint8 _noun;
		uint8 _verb;
		uint8 _kase;
		uint8 _startSeq;
		uint8 _endSeq;
		uint8 _curSeq;

		Audio::SoundHandle _handle;
		Common::String _msg;

		SpeakingState(const uint16 catalog, const uint8 talker, const uint8 noun, const uint8 verb, const uint8 kase, const uint8 startSeq, const uint8 endSeq);
		void start();
		bool update();
	};

public:
	static Object *factory(const KQFile &f);
	Monster(const KQFile &f);
	virtual void addToInventory(const ItemType *itemType, uint16 quantity);
	virtual void removeFromInventory(ItemType *itemType, uint16 quantity = 1);
	void speak(uint16 catalog, uint8 noun, uint8 verb, uint8 kase, uint8 startSeq, uint8 endSeq);
	void update(float dt) override;
	void setAlarmRadius(float alarmRadius) { _alarmRadius = alarmRadius; }
	void setChaseRadius(float chaseRadius) { _chaseRadius = chaseRadius; }
	void setHome(const Math::Vector3d &pos) { _homePosition = pos; }
	void setHomeRadius(float homeRadius) { _homeRadius = homeRadius; }

protected:
	Common::HashMap<const ItemType *, uint16> _inventory;
	Common::List<SpeakingState> _speaking;
	Math::Vector3d _homePosition;
	float _alarmRadius;
	float _chaseRadius;
	float _homeRadius;
	Math::Vector3d _speed;
};

} // namespace Kq8

#endif // KQ8_OBJECTS_ MONSTER_H
