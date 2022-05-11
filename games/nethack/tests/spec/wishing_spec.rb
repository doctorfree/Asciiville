#!/usr/bin/env ruby

require 'lib/unnethack'

def wish_for(wish, check)
	wished = @u.wish_for wish
	wished.should =~ check
end

describe UnNetHack do
	before(:all) do
		@u = UnNetHack.new
	end

	after(:all) do
		@u.quit rescue nil
	end

	it "should return the correct dragon scales" do
		wish_for "draken scales", /draken scales/
		wish_for "draken dragon scales", /draken scales/
		wish_for "lindworm scales", /lindworm scales/
		wish_for "lindworm dragon scales", /lindworm scales/
		wish_for "sarkany scales", /sarkany scales/
		wish_for "sarkany dragon scales", /sarkany scales/
		wish_for "sirrush scales", /sirrush scales/
		wish_for "sirrush dragon scales", /sirrush scales/
		wish_for "leviathan scales", /leviathan scales/
		wish_for "leviathan dragon scales", /leviathan scales/
		wish_for "wyvern scales", /wyvern scales/
		wish_for "wyvern dragon scales", /wyvern scales/
		wish_for "guivre scales", /guivre scales/
		wish_for "guivre dragon scales", /guivre scales/
		wish_for "glowing dragon scales", /glowing dragon scales/
	end
end
