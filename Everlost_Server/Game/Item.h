#ifndef GAME_ITEM_H
#define GAME_ITEM_H

class Item
{
public:
	virtual ~Item();

protected:
	Item();

private:
	Item(const Item& _other);
	Item& operator=(const Item& _other);
};

#endif