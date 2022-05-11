/*	SCCS Id: @(#)monattk.h	3.4	2002/03/24	*/
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */

#ifndef MONATTK_H
#define MONATTK_H

/*	Add new attack types below - ordering affects experience (exper.c).
 *	Attacks > AT_BUTT are worth extra experience.
 */
#define AT_ANY		(-1)	/* fake attack; dmgtype_fromattack wildcard */
#define AT_NONE		0	/* passive monster (ex. acid blob) */
#define AT_CLAW		1	/* claw (punch, hit, etc.) */
#define AT_BITE		2	/* bite */
#define AT_KICK		3	/* kick */
#define AT_BUTT		4	/* head butt (ex. a unicorn) */
#define AT_TUCH		5	/* touches */
#define AT_STNG		6	/* sting */
#define AT_HUGS		7	/* crushing bearhug */
#define AT_SPIT		10	/* spits substance - ranged */
#define AT_ENGL		11	/* engulf (swallow or by a cloud) */
#define AT_BREA		12	/* breath - ranged */
#define AT_EXPL		13	/* explodes - proximity */
#define AT_BOOM		14	/* explodes when killed */
#define AT_GAZE		15	/* gaze - ranged */
#define AT_TENT		16	/* tentacles */
#define AT_SCRE		17	/* scream - sonic attack */

#define AT_WEAP		254	/* uses weapon */
#define AT_MAGC		255	/* uses magic spell(s) */

/*	Add new damage types below.
 *
 *	Note that 1-10 correspond to the types of attack used in buzz().
 *	Please don't disturb the order unless you rewrite the buzz() code.
 */
#define AD_ANY		(-1)	/* fake damage; attacktype_fordmg wildcard */
#define AD_PHYS		0	/* ordinary physical */
#define AD_MAGM		1	/* magic missiles */
#define AD_FIRE		2	/* fire damage */
#define AD_COLD		3	/* frost damage */
#define AD_SLEE		4	/* sleep ray */
#define AD_DISN		5	/* disintegration (death ray) */
#define AD_ELEC		6	/* shock damage */
#define AD_DRST		7	/* drains str (poison) */
#define AD_LAVA		8	/* a jet of molten lava */
#define AD_ACID		9	/* acid damage */
#define AD_SPC1		10	/* for extension of buzz() */
#define AD_SPC2		11	/* for extension of buzz() */
#define AD_BLND		12	/* blinds (yellow light) */
#define AD_STUN		13	/* stuns */
#define AD_SLOW		14	/* slows */
#define AD_PLYS		15	/* paralyses */
#define AD_DRLI		16	/* drains life levels (Vampire) */
#define AD_DREN		17	/* drains magic energy */
#define AD_LEGS		18	/* damages legs (xan) */
#define AD_STON		19	/* petrifies (Medusa, cockatrice) */
#define AD_STCK		20	/* sticks to you (mimic) */
#define AD_SGLD		21	/* steals gold (leppie) */
#define AD_SITM		22	/* steals item (nymphs) */
#define AD_SEDU		23	/* seduces & steals multiple items */
#define AD_TLPT		24	/* teleports you (Quantum Mech.) */
#define AD_RUST		25	/* rusts armour (Rust Monster)*/
#define AD_CONF		26	/* confuses (Umber Hulk) */
#define AD_DGST		27	/* digests opponent (trapper, etc.) */
#define AD_HEAL		28	/* heals opponent's wounds (nurse) */
#define AD_WRAP		29	/* special "stick" for eels */
#define AD_WERE		30	/* confers lycanthropy */
#define AD_DRDX		31	/* drains dexterity (quasit) */
#define AD_DRCO		32	/* drains constitution */
#define AD_DRIN		33	/* drains intelligence (mind flayer) */
#define AD_DISE		34	/* confers diseases */
#define AD_DCAY		35	/* decays organics (brown Pudding) */
#define AD_SSEX		36	/* Succubus seduction (extended) */
				/* If no SEDUCE then same as AD_SEDU */
#define AD_HALU		37	/* causes hallucination */
#define AD_DETH		38	/* for Death only */
#define AD_PEST		39	/* for Pestilence only */
#define AD_FAMN		40	/* for Famine only */
#define AD_SLIM		41	/* turns you into green slime */
#define AD_ENCH		42	/* remove enchantment (disenchanter) */
#define AD_CORR		43	/* corrode armor (black pudding) */
#define AD_LUCK		44	/* drain luck (evil eye) */
#define AD_FREZ		45	/* freezing attack (blue slime) */
#define AD_HEAD		46	/* decapitate (vorpal jabberwock) */

#define AD_PUNI		47	/* punisher spells */

#define AD_LVLT		48	/* level teleport (weeping angel) */
#define AD_BLNK		49	/* mental invasion (weeping angel) */

#define AD_SPOR		50	/* generate spore */

#define AD_CLRC		240	/* random clerical spell */
#define AD_SPEL		241	/* random magic spell */
#define AD_RBRE		242	/* random breath weapon */

#define AD_SAMU		252	/* hits, may steal Amulet (Wizard) */
#define AD_CURS		253	/* random curse (ex. gremlin) */


/*
 *  Monster to monster attacks.  When a monster attacks another (mattackm),
 *  any or all of the following can be returned.  See mattackm() for more
 *  details.
 */
#define MM_MISS		0x0	/* aggressor missed */
#define MM_HIT		0x1	/* aggressor hit defender */
#define MM_DEF_DIED	0x2	/* defender died */
#define MM_AGR_DIED	0x4	/* aggressor died */

#endif /* MONATTK_H */
