#!/usr/bin/perl

# Dungeon names
%dungeons = ( 0 => "the Dungeons of Doom",
              1 => "Gehennom",
              2 => "the Gnomish Mines",
              3 => "the Quest",
              4 => "Sokoban",
              5 => "Fort Ludios",
              6 => "Vlad's Tower",
              7 => "the Elemental Planes" );

# Conducts, in the order that they are stored in the conduct bitfield
@conducts = ( "Foodless", "Vegan", "Vegetarian", "Atheist", "Weaponless",
              "Pacifist", "Illiterate", "Polypileless", "Polyselfless",
              "Wishless", "Artifact Wishless", "Genocideless" );

# Achievements, in the order that they are stored in the achievement
# bitfield
@achievements = ( "Obtained the Bell of Opening", 
                  "Entered Gehennom",
                  "Obtained the Candelabrum of Invocation",
                  "Obtained the Book of the Dead",
                  "Performed the invocation ritual",
                  "Obtained the Amulet of Yendor",
                  "Reached the Elemental Planes",
                  "Reached the Astral Plane",
                  "Ascended",
                  "Completed the Mines",
                  "Completed Sokoban",
                  "Killed Medusa" );
                 
while($entry = <>) {
  # Parse the line and store in the hash %field
  @fields = split /:/, $entry;
  
  foreach $field (@fields) {
    if($field =~ /^([^=]*)=(.*)$/) {
      $fname = $1;
      $fval = $2;
      $field{$fname} = $fval;
    }
  }

  # Display
  printf "%s-%s-%s-%s-%s, %s\n", $field{name}, 
          $field{role}, $field{race}, $field{gender}, $field{align},
          $field{death};
  printf "    Died in %s on level %d (max %d).  Final HP %d/%d.\n",
          $dungeons{$field{deathdnum}}, $field{deathlev}, $field{maxlvl},
          $field{hp}, $field{maxhp};
          
  @c = ();
  $field{conduct} = oct $field{conduct};
  for($i = 0; $i <= $#conducts; $i++) {
    if($field{conduct} & (1 << $i)) {
      push @c, $conducts[$i];
    }
  }
  print  "    Conducts: ", join(', ', @c), "\n";

  @a = ();
  $field{achieve} = oct $field{achieve};
  for($i = 0; $i <= $#achievements; $i++) {
    if($field{achieve} & (1 << $i)) {
      push @a, $achievements[$i];
    }
  }
  print  "    Notable achievements: ", join(', ', @a), "\n";


  printf "    The game lasted %d turns, and took %d seconds of playtime.\n",
          $field{turns}, $field{realtime};
}
