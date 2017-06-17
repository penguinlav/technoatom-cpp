#include "Level.h"

#include <assert.h>
#include <iostream>

#include "../Logger/Logger.h"

using IlluminatiConfirmed::Object;
using IlluminatiConfirmed::Layer;
using IlluminatiConfirmed::Level;

void Level::loadMapInfoFromFile(tinyxml2::XMLDocument &levelFile) {
  // Работаем с контейнером map
  tinyxml2::XMLElement *map;
  map = levelFile.FirstChildElement("map");

  m_width = std::stoi(map->Attribute("width"));
  m_height = std::stoi(map->Attribute("height"));
  m_tileWidth = std::stoi(map->Attribute("tilewidth"));
  m_tileHeight = std::stoi(map->Attribute("tileheight"));

  // Берем описание тайлсета и идентификатор первого тайла
  tinyxml2::XMLElement *tilesetElement;
  tilesetElement = map->FirstChildElement("tileset");
  m_firstTileId = atoi(tilesetElement->Attribute("firstgid"));

  // source - путь до картинки в контейнере image
  tinyxml2::XMLElement *image;
  image = tilesetElement->FirstChildElement("image");
  m_name_of_tileset = image->Attribute("source");

  m_columns = std::stoi(image->Attribute("width")) / m_tileWidth;
  m_rows = std::stoi(image->Attribute("height")) / m_tileHeight;
}

void Level::loadLayersFromFile(tinyxml2::XMLDocument &levelFile) {
  std::vector<sf::Rect<int>> sub_rects;
  sub_rects.reserve(m_rows * m_columns);
  for (int y = 0; y < m_rows; y++)
    for (int x = 0; x < m_columns; x++) {
      sub_rects.push_back(sf::Rect<int>(x * m_tileWidth, y * m_tileHeight,
                                        m_tileWidth, m_tileHeight));
    }

  // Работа со слоями
  tinyxml2::XMLElement *map;
  map = levelFile.FirstChildElement("map");

  tinyxml2::XMLElement *layerElement;
  layerElement = map->FirstChildElement("layer");
  while (layerElement) {
    // Layer layer;
    int opacity;
    std::string name_layer;

    // Если присутствует opacity, то задаем прозрачность слоя, иначе он
    // полностью непрозрачен

    if (layerElement->Attribute("name") != NULL) {
      name_layer = layerElement->Attribute("name");
    } else {
      LOG() << "Map doesn't have layer name \n";
      // assert (!"Map doesn't have layer name \n");
    }

    if (layerElement->Attribute("opacity") != NULL) {
      opacity = int(255 * std::stof(layerElement->Attribute("opacity")));
    } else {
      opacity = 255;
    }

    // Контейнер <data>
    tinyxml2::XMLElement *layerDataElement;
    layerDataElement = layerElement->FirstChildElement("data");

    if (layerDataElement == NULL) {
      LOG() << "Bad map. No layer information found.\n";
    }

    // Контейнер <tile> - описание тайлов каждого слоя
    tinyxml2::XMLElement *tileElement;
    tileElement = layerDataElement->FirstChildElement("tile");

    if (tileElement == NULL) {
      LOG() << "Bad map. No tile information found.\n";
    }

    int x = 0;
    int y = 0;
    std::map<int, std::pair<sf::Rect<int>, sf::Vector2i>> sub_rects_layer;
    int cell = 0;

    while (tileElement) {
      int tileGID = std::stoi(tileElement->Attribute("gid"));

      int sub_rect_to_use = tileGID - m_firstTileId;

      // Устанавливаем TextureRect каждого тайла
      if (sub_rect_to_use >= 0) {
        if (sub_rects.size() > tileGID) {
          sub_rects_layer.insert(
              {cell,
               {sub_rects.at(sub_rect_to_use),
                {x * m_tileWidth, y * m_tileHeight}}});  // position
          // sub_rects_layer.push_back({sub_rects.at(sub_rect_to_use),
          //                           {x * m_tileWidth, y * m_tileHeight}});
        } else {
          LOG() << "Something wrong with tile element: "
                << tileElement->Attribute("gid") << " on layer "
                << layerElement->Attribute("name") << ".\n";
        }
      }
      tileElement = tileElement->NextSiblingElement("tile");

      x++;
      if (x >= m_width) {
        x = 0;
        y++;
        if (y >= m_height) y = 0;
      }
      ++cell;
    }
    m_layers.push_back(
        {opacity, std::move(name_layer), std::move(sub_rects_layer)});

    layerElement = layerElement->NextSiblingElement("layer");
  }

}

void Level::loadObjectsFromFile(tinyxml2::XMLDocument &levelFile) {
  // Работа с объектами
  tinyxml2::XMLElement *objectGroupElement;

  tinyxml2::XMLElement *map;
  map = levelFile.FirstChildElement("map");

  // Если есть слои объектов
  if (map->FirstChildElement("objectgroup") != NULL) {
    objectGroupElement = map->FirstChildElement("objectgroup");
    while (objectGroupElement) {
      // Контейнер <object>
      tinyxml2::XMLElement *objectElement;
      objectElement = objectGroupElement->FirstChildElement("object");

      while (objectElement) {
        // Получаем все данные - тип, имя, позиция, etc
        std::string objectType;
        if (objectElement->Attribute("type") != NULL) {
          objectType = objectElement->Attribute("type");
        }
        std::string objectName;
        if (objectElement->Attribute("name") != NULL) {
          objectName = objectElement->Attribute("name");
        }
        int x = std::stoi(objectElement->Attribute("x"));
        int y = std::stoi(objectElement->Attribute("y"));

        int width = 0, height = 0;

        if (objectElement->Attribute("width") != NULL) {
          width = atoi(objectElement->Attribute("width"));
          height = atoi(objectElement->Attribute("height"));
        } else
          LOG() << "Object doesn't have width and height" << std::endl;

        // Экземпляр объекта
        Object object = {std::move(objectName), std::move(objectType),
                         sf::Rect<int>(x, y, width, height)};

        // Пихаем объект в вектор
        m_objects.push_back(std::move(object));

        objectElement = objectElement->NextSiblingElement("object");
      }
      objectGroupElement =
          objectGroupElement->NextSiblingElement("objectgroup");
    }
  } else {
    LOG() << "No object layers found...\n";
  }
}

void Level::loadMapFromFile(const std::string &filename) {
  tinyxml2::XMLDocument levelFile;
  // TiXmlDocument levelFile(filename.c_str());

  // Загружаем XML-файл с картой
  if (levelFile.LoadFile(filename.c_str())) {
    LOG() << "Loading level \"" << filename
          << "\" failed with message: " << levelFile.ErrorName() << "\n";
  }

  loadMapInfoFromFile(levelFile);  // загружаем основную инфу
  loadLayersFromFile(levelFile);   // загружаем слои
  loadObjectsFromFile(levelFile);  // загружаем объекты
}

Object Level::GetObject(const std::string &name) {
  // Только первый объект с заданным именем
  for (auto &&it : m_objects)
    if (it.m_name == name) return it;
  throw EXCEPTION(std::string("Unknown object ") + name, nullptr);
}

std::vector<Object> Level::GetObjectsByName(const std::string &name) {
  // Все объекты с заданным именем
  std::vector<Object> vec;
  for (auto &&it : m_objects)
    if (it.m_name == name) vec.push_back(it);
  return vec;
}

std::vector<Object> Level::GetObjectsByType(const std::string &type) {
  // Все объекты с заданным именем
  std::vector<Object> vec;
  for (auto &&it : m_objects)
    if (it.m_type == type) vec.push_back(it);
  return vec;
}

const Layer &Level::GetLayerByName(const std::string &name) {
  for (auto &&it : m_layers) {
    if (it.m_name == name) return it;
  }
  throw EXCEPTION(std::string("Unknown layer ") + name, nullptr);
}

sf::Vector2i Level::GetTileSize() {
    return sf::Vector2i(m_tileWidth, m_tileHeight);
}

IlluminatiConfirmed::MapInfo Level::GetMapInfo()
{
    return { {m_tileWidth, m_tileHeight}, {m_width, m_height},m_firstTileId, m_columns,
                m_rows };
}

std::vector<std::pair<sf::Rect<int>, sf::Vector2i>> Level::GetVecOfRectsByNameOfObjAndLayer(
    const std::string &name_obj, const std::string &name_layer) {
  auto rect = GetObject(name_obj).m_rect;
  sf::Vector2i vertex[] = {{rect.left, rect.top},
                           {rect.left + rect.width, rect.top},
                           {rect.left, rect.top + rect.height},
                           {rect.left + rect.width, rect.top + rect.height}};

  auto sub_rects = GetLayerByName(name_layer).m_sub_rects;

  std::vector<std::pair<sf::Rect<int>, sf::Vector2i>> vec_of_rects;

  for (int j = int(vertex[0].y / m_tileHeight);
       j <= int(vertex[2].y / m_tileHeight); ++j) {
    for (int i = int(vertex[0].x / m_tileWidth);
         i <= int(vertex[1].x / m_tileWidth); ++i) {
      vec_of_rects.push_back(sub_rects.at(j * m_width + i));
    }
  }
  return vec_of_rects;
}

/*
void Level::Draw(sf::RenderWindow &window) {
  // Рисуем все тайлы (объекты НЕ рисуем!)
  for (auto &&layer : m_layers)
    for (auto &&tile : layer.m_tiles) window.draw(tile);
}
*/
