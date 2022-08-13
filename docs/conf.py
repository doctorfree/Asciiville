# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Asciiville'
copyright = '2022, Ronald Record'
author = 'Ronald Record'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

# tmuxp uses all these extensions
#extensions = [
#   "sphinx.ext.autodoc",
#   "sphinx.ext.intersphinx",
#   "sphinx_autodoc_typehints",
#   "sphinx.ext.todo",
#   "sphinx.ext.napoleon",
#   "sphinx.ext.linkcode",
#   "aafig",
#   "sphinx_issues",
#   "sphinx_click.ext",  # sphinx-click
#   "sphinx_inline_tabs",
#   "sphinx_copybutton",
#   "sphinxext.opengraph",
#   "sphinxext.rediraffe",
#   "myst_parser",
#]

extensions = ['myst_parser']

myst_enable_extensions = ["colon_fence", "substitution", "replacements"]

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

source_suffix = {".rst": "restructuredtext", ".md": "markdown"}

master_doc = "index"

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ['_static']
