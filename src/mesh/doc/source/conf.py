# -*- coding: utf-8 -*-

import sys, os


extensions = ["sphinx.ext.imgmath"]

templates_path = ["_templates"]

source_suffix = ".rst"


master_doc = "mesh"

project = "ns-3"
copyright = "ns-3 project"

version = "ns-3-dev"
release = "ns-3-dev"


exclude_patterns = []


pygments_style = "sphinx"


html_theme = "default"


latex_documents = [
    (
        "mesh",
        "mesh-module-doc.tex",
        "The ns-3 Mesh Wi-Fi Module Documentation",
        "ns-3 project",
        "manual",
    ),
]


pdf_break_level = 4


man_pages = [("index", "ns-3-model-library", "ns-3 Model Library", ["ns-3 project"], 1)]
