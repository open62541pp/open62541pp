# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

# -- Run Doxygen on Read the Docs server -------------------------------------

import subprocess, os

def configure_doxyfile(input_dir, output_dir):
    with open('Doxyfile.in', 'r') as file:
        filedata = file.read()

    filedata = filedata.replace('@DOXYGEN_INPUT_DIR@',  input_dir)
    filedata = filedata.replace('@DOXYGEN_OUTPUT_DIR@', output_dir)
    
    with open('Doxyfile', 'w') as file:
        file.write(filedata)

# Check if script is executed on a Read the Docs' server
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

breathe_projects = {}
if read_the_docs_build:
    input_dir  = '../include/open62541pp'
    output_dir = 'build'
    configure_doxyfile(input_dir, output_dir)
    subprocess.call('doxygen', shell=True)
    breathe_projects['open62541pp'] = output_dir + '/xml'


# -- Project information -----------------------------------------------------

project   = 'open62541pp'
copyright = '2019, Lukas Berbuer'
author    = 'Lukas Berbuer'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['breathe', 'm2r']

# Add any paths that contain templates here, relative to this directory.
# templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The default language to highlight source code in.
highlight_language = 'cpp'


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']

# -- Breathe configuration ---------------------------------------------------
breathe_default_project = "open62541pp"
breathe_default_members = ('members', 'undoc-members')