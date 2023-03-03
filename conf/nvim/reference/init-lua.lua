-- References to ./lua/

-- Load global functions
require("globals")
-- Plugin management via lazy
require("lazy-init")
-- Plugin management via Packer
-- require("packer")
-- "Global" Keymappings
require("mappings")
-- All non plugin related (vim) options
require("options")
-- Vim autocommands/autogroups
require("autocmd")
