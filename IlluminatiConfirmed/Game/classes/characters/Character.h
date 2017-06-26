#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
//#include "Weapons.h"
#include "../../Logger/Logger.h"
#include "Base.h"
#include "Box2D/Box2D.h"
#include "constants.h"

#define DEBUG_ON
#if defined(DEBUG_ON)
#define LOG() std::cout
#else
#define LOG() LOGGER("level")
#endif

namespace IlluminatiConfirmed {
namespace experimental {

class Weapon;

struct CharacterSpriteInfo {
  int width;
  int height;
  int size;
  int count_of_frames;
  int x_position;
  int y_position;
};

template <class PlaySound>
struct SoundPackTCharacter {
  std::shared_ptr<PlaySound> killed;
};

template <>
struct SoundPackTCharacter<QSound> {
  SoundPackTCharacter(std::shared_ptr<QSound> &&hit) : killed(hit) {}
  std::shared_ptr<QSound> killed;
};

class BaseCharacter : public BaseInterface {
 public:
  enum TypeBaseCharacter { CHARACTER_SOUTH_PARK, ALINAS_BOYS };
  using SoundPack = SoundPackTCharacter<IlluminatiPlaySound>;

  //передавать лямбду, которая будет дергаться, когда персу необходимо будет
  //стрелять
  BaseCharacter(b2World *world, const sf::Texture *texture,
                const CharacterSpriteInfo &sprite_data, SoundPack pack);

  virtual void draw(sf::RenderWindow &window) override;
  virtual void move(b2Vec2 velocity, float deltaTime) override;
  virtual void contact(BaseInterface *B) override;
  virtual void endContact(BaseInterface *B) override;

  void setWeapon(std::unique_ptr<Weapon> &&weapon);
  void moveWeapon(const sf::Vector2f &pos, float rot);
  void setAngleOfWeapon(float angle);

  virtual void attack();

  void playKilled() {
    if (m_pack.killed) m_pack.killed->play();
  }

  virtual ~BaseCharacter();

  virtual void updatePhysics(float deltaTime);

  TypeBaseCharacter m_type_character;

 protected:
  int m_frames;
  Direction m_direction;
  //  std::vector<sf::Rect<int>> front_rects;
  //  std::vector<sf::Rect<int>> back_rects;
  //  std::vector<sf::Rect<int>> left_rects;
  //  std::vector<sf::Rect<int>> right_rects;
  std::map<Direction, std::vector<sf::Rect<int>>>
      m_directionRects;  // ыыыыыыы....

  b2Body *m_b2_body;
  b2Fixture *m_b2_body_fixture;

  b2Body *m_b2_weapon;
  b2Fixture *m_2b_weapon_fixture;

  sf::Sprite m_sprite;

 private:
  std::unique_ptr<Weapon> m_weapon;
  float m_height;
  SoundPack m_pack;
};

class CharacterSouthPark : public BaseCharacter {
 public:
  CharacterSouthPark(b2World *world, const sf::Texture *texture,
                     const CharacterSpriteInfo &sprite_data, SoundPack pack);
  void move(b2Vec2 velocity, float deltaTime) override;
  void draw(sf::RenderWindow &window) override;
  void contact(BaseInterface *B) override;
  void endContact(BaseInterface *B) override;
  ~CharacterSouthPark();

  b2MotorJoint *m_b2_joint;
  b2PrismaticJoint *m_b2_joint_prism;

  void updatePhysics(float deltaTime);
};

class CharacterAlinasBoys : public BaseCharacter {
 public:
  CharacterAlinasBoys(b2World *world, const sf::Texture *texture,
                      const CharacterSpriteInfo &sprite_data, SoundPack pack);

  void move(b2Vec2 velocity, float deltaTime) override;
  void draw(sf::RenderWindow &window) override;
  void contact(BaseInterface *B) override;
  void endContact(BaseInterface *B) override;
  ~CharacterAlinasBoys();
};
}
}
