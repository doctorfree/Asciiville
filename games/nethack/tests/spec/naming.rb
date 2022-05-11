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

	it "should not crash when calling and naming twice an unidentified object with the largest strings" do
		# object must be unidentified, nobody starts with the knowledge about BoH
		object = @u.wish_for "bag of holding"
		slot = object[0..0]
		@u.call slot, "1"*80
		@u.name slot, "2"*80
		@u.name slot, "3"*80
		# program will only crashes if compiled with buffer overflow detection
	end
end
