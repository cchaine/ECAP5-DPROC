import os, sys

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'ECAP5-DPROC'
copyright = '2024, Clément Chaine'
author = 'Clément Chaine'
release = '1.0.0-alpha1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

sys.path.append(os.path.abspath('./_ext'))

extensions = [
    'sphinx_rtd_theme', 
    'sphinx.ext.todo',
    'requirement']
templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_logo = 'assets/logo.svg'
html_theme_options = {
    'display_version': True,
    'collapse_navigation': False,
    'titles_only': False,
    'navigation_depth': 5,
    'style_external_links': True
}
html_css_files = [
    'css/custom.css'
]


todo_include_todos = True
todo_emit_warnings = True
