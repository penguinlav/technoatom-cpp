#pragma once

#include "Box2D/Box2D.h"
#include "SFML/Graphics.hpp"

#include "Bullet.h"
#include "Event/Event.h"
#include "Factories.h"

namespace IlluminatiConfirmed {
namespace experimental {

struct WeaponInfo {
  TypeBullet bullet_type;
  sf::Rect<int> rect_weapon;
  int count_of_rect;
  int number_of_cartridge;  // damage в пульке
  float scale;
};

class SpriteInterface {
  virtual void draw(sf::RenderWindow *) = 0;
};

struct BulletSetsInfo {
  BulletSets sets;
  TypeBullet type;
  BaseCharacter *whose;
};

class Weapon {
 public:
  Weapon(sf::Texture *texture, const WeaponInfo &info);

  void setPositionRotation(const sf::Vector2f &pos, float rotation);
  // void setWhose(BaseCharacter * who);

  void attack(BaseCharacter *who);

  void draw(sf::RenderWindow &window);

  Event<BulletSetsInfo> event_create_bullet;

  virtual ~Weapon() {}

 private:
  struct TimeAnimation {
    int time;
    int start_time;
    int count;
  };

  void initTimeAnimation();
  sf::Sprite m_sprite;
  int m_number_of_cartridge;

  std::vector<sf::Rect<int>> m_rects_weapon;

  TimeAnimation m_animation;
  TypeBullet m_type_bullet;
  BaseCharacter *m_whose;
};

class ListnerWeapon {
 public:
  ListnerWeapon();

  void setPointers(
      b2World *world,
      std::vector<std::shared_ptr<experimental::BulletInterface>> *bullets,
      std::vector<std::shared_ptr<experimental::BaseInterface>> *objs);

  void addWeapon(Weapon *class_);

  void pushBullet(BulletSetsInfo &&bullet_sets);
  std::vector<
      std::pair<SyncValue<BulletSetsInfo>, EventListener<BulletSetsInfo>>>
      m_syncValues_and_eventListener;

 private:
  b2World *m_world;
  std::vector<std::shared_ptr<experimental::BulletInterface>> *m_bullets;
  std::vector<std::shared_ptr<experimental::BaseInterface>> *m_objs;
};
}
}
