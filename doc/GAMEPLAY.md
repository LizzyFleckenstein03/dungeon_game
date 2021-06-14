# Gameplay

## The Basics

You are in a pixely dungeon. There is no exit, the goal of the game is to survive and to collect XP (score). There are monsters and items in the world; you have to collect things and defeat monsters.

## Controls
You can use WASD to move up / left / down / right. Use Q to quit and Space to shoot fireballs if you have them in your inventory.
To navigate the inventory, use the arrow keys (up and down) to select an item and then press enter to use it.

## The map

The map consists of nodes and entities. The nodes are static; they cannot move and you cannot break or replace them. They are automatically generated on startup. Some nodes are solid, which means entities collide with them, others are not.
Entities are things that can move or be removed / spawned dynamically. The player is an entity themselves. Some of them collide with other entities, others don't.
The player emits light that has a radius of 10. You can't see anything outside of this radius.

### Nodes

There are two types of nodes, wall and floor. Entities collide with walls and do not collide with floor.

## Entities

#### Food

There are some collectable food sources, like apples and cherries. Collecting them gives you XP (apples 1, cherries 2). The main difference between apples and cherries is that the player will eat apples immediately, while cherries will be collected and go into the inventory. You can then use them to eat them. Apples heal 1 heart, Cherries 2.

#### Monsters

Some of the entities are monsters. There is currently only one type of entity, the alien monster. It has a very basic AI and cannot walk around obstacles. It walks 1 node per 0.5 seconds and also attacks at that rate. An alien monster attack deals one heart of damage. Killing a monster gives you 5 XP.

#### Projectiles

If you have fireballs in your inventory, you can shoot them (See Controls) or selecting them in your inventory and using them. They are shot into the last direction you moved or attempted to move into. Fireballs move at a speed of 1 node per 0.1 seconds and deal 3 - 5 hearts of damage to any entity they collide with (they are removed on collision).

## Inventory

The inventory is a list of things you carry on you. Some items can be used, meaning a specific action depending on the type of item is performed and they may be consumed by using them.

### Items

#### Cherry

Cherries are consumed when used. They are stackable. See Food for an explination.

#### Fireballs

Fireballs are consumed when used. They are stackable. See Projectiles for an explination.

#### Swords

Swords are not consumed when used, but they have a 1 / 100 chance to turn into a Broken Sword. They are not stackable. They need to recharge for 1 second before being used again. They hit any entity that collides with other entities and stands directly next to you in the direction you last moved or attempted to move into.

#### Broken Swords

Broken Swords are consumed / destroyed when used, but using them does nothing else. They are not stackable and they currently have no real purpose.

## UI

### The health bar

In the lower center of your screen you can see 10 hearts. Depending on whether they are red or gray they are active (red) or gone (gray). If you have 0 hearts you will die (game over). You screen turns red every time you take damage.

### The recharge meter

If you use an item that needs to recharge after being used, a recharge meter will show. It tells you the recharge percentage and visualizes it using a bar. You cannot use

### The score count

In the upper light corner there is a display that tells you your current score.

### The inventory

The inventory is rendered at the left half of the screen. It tells you which items you have; some items can be stacked, meaning e.g. a stack of 7 cherries will only be one entry in the list. However if an entry in the list is a stack, it will display the count after the name.
There is also an arrow in front of the currently selected item (See Controls)
