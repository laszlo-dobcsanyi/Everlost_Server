#include "Core\Macro.h"
#include "Game\World.h"

#include <fstream>

#include <boost\lexical_cast.hpp>

#include "Game\Object.h"
#include "Game\Field_Loaded.h"
#include "Game\Field_Generated.h"

/// PUBLIC

World::World(const int& _id)
	: id(_id)
{
	std::ifstream world_file("worlds\\" + boost::lexical_cast<std::string>(_id) + ".world");
	if(world_file)
	{
		world_file >> name >> width >> height;

		#ifdef LOGGING
		Logger::Write(LogMask::initialize, LogObject::world, "\t> Loading World (" + boost::lexical_cast<std::string>(id) + ")..");
		#endif

		//Create fields table
		fields = new Field_Loaded*[width * height];
		for(int current = 0; current < width * height; ++current) fields[current] = 0;

		//Process areas
		int areas_number; world_file >> areas_number;
		while(areas_number--)
		{
			//Process current camp
			int area_x, area_y, area_fields; world_file >> area_x >> area_y >> area_fields;
			while(area_fields--)
			{
				//Process current field
				int field_x, field_y, objects; world_file >> field_x >> field_y >>objects;
				fields[(area_x + field_x) + (area_y + field_y) * width] = new Field_Loaded();

				while (objects--)
				{
					float object_x, object_y; int object_id; world_file >> object_x >> object_y >> object_id;
					fields[(area_x + field_x) + (area_y + field_y) * width]->objects.push_back(new Object((area_x + field_x) * 512.f + object_x, (area_y + field_y) * 512.f + object_y, object_id));
				}
			}

		}
	}
	else
	{
		#ifdef LOGGING
		Logger::Write(LogMask::fatal_error, LogObject::world, "# Error @ World constructor -> World file (" + boost::lexical_cast<std::string>(id) + ") not found!");
		#endif
	}
}

Field* World::GetField(const int& _x, const int& _y) const
{
	if (fields[_x + _y * width]) return fields[_x + _y * width];
	return new Field_Generated();
}