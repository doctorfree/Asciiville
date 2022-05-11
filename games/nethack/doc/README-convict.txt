Convict role patch v0.7 for NetHack 3.4.3
Karl Garrison <kgarrison@pobox.com>


Non-spoiler Information
~~~~~~~~~~~~~~~~~~~~~~~

This patch adds a new role to NetHack: the Convict.  The Convict is an
escaped prisoner who seeks to redeem himself by seeking the Amulet in
the Dungeons of Doom.  They start out with a striped shirt, some rocks,
and heavy iron ball chained to them.  A trusty sewer rat accompanies
the Convict into the dungeon: a companion from his days of
imprisonment.  Convicts have an affinity with rats, but many other
animals do not trust him.

Race: Human, Dwarf, Gnome, or Orc
Alignment: Chaotic
Deity: Tymora (Forgotten Realms)

New configuration file option: ratname.  Example:

OPTIONS=ratname:Squeak













Spoiler Information
~~~~~~~~~~~~~~~~~~~

There may be a few surprises to this role for the unaware.  They start
with a negative alignment ("You have transgressed").  This means that a
Convict will not be able to pray successfully early on.  Some time and
killing of monsters will make up for this before too long, though.
They also start the game on the verge of hungry, which also makes the
very early game challenging for Convicts.  On the bright side, it takes
them longer to progress from hungry to weak, etc, due to their higher
tolerance for irregular meals.

Stats: Excellent CON, good STR, average DEX and INT, poor WIS and CHA.

Intrinsics: Sick resistance at level 1 (due to exposure to unhealthy
prison conditions, as well as bad food), poison resistance at level 7,
search at level 20.

Pet: The convict starts with a sewer rat, which quickly advances to a
giant rat, and can eventually become an enormous rat, then a rodent of
unusual size.  Convicts can tame rats with food, but not domestic
amimals like dogs and cats.  The latter are instead made peaceful.

Convicts can also usually make rats peaceful and occasionally tame by
#chatting with them.

Iron Ball: The Convict starts with a heavy iron ball chained to him, as
if he had read a scroll of punishment.  Since he starts with no real
weapons apart from his rocks, he may wish to wield it as a weapon.
Convicts (and only Convicts) will practice the flail skill when he
weilds an iron ball.  Advancing flail skill improves his chance to hit,
and prevents him from automatically falling down the stairs if one is
chained to him, so long as he is currently wielding it.

Striped Shirt: Shopkeepers who can see the Convict wearing his striped
shirt will recognize the player as an escaped prisoner, and will ban
him from the shop.  Even if he later hides the shirt, that shopkeeper
will remember the player.

Guards: Guards (town watch and vault guards) who see a Convict will
recognize him from wanted posters, and immediately become hostile.

Hunger: Convicts start on the verge of hungry, but the effects of
hunger kick in a bit more slowly for this role, due to their learned
tolerance of inadequate nutrition.

Weapon Skills:

Basic: mace, short sword

Skilled: dagger, dart, hammer, sling, two-weapon combat
bare-handed combat

Expert: club, flail, knife, pick-axe


Spellcasting Skills:

Basic: Matter

Expert: Escape


Luck Sword: This is a Chaotic broadsword which acts as a luckstone, and
is the Convict's first sacrifice gift.

The Quest: The Convict quest has the player return to Castle
Waterdeep's dungeons in order to retrieve the Iron Ball of Liberation
from the corrupted Warden Arianna on behalf of their quest leader,
Robert the Lifer.

The Iron Ball of Liberation: This is a (slightly lighter) heavy iron
ball that grants magic resistance, stealth, searching and warning.
When invoked, it allows phasing for a limited time (the ability to
walk through walls/stone like a xorn).  The downsides are its weight
(almost as heavy as a normal iron ball) and the fact that it chains
itself to the player everytime its power is invoked.

