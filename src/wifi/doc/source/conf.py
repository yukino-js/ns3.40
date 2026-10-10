# -*- coding: utf-8 -*-

import sys, os


extensions = ["sphinx.ext.imgmath"]

templates_path = ["_templates"]

source_suffix = ".rst"


master_doc = "wifi"

project = "ns-3"
copyright = "ns-3 project"

version = "ns-3-dev"
release = "ns-3-dev"


exclude_patterns = []


pygments_style = "sphinx"


html_theme = "default"


latex_documents = [
    (
        "wifi",
        "wifi-module-doc.tex",
        "The ns-3 Wi-Fi Module Documentation",
        "ns-3 project",
        "manual",
    ),
]


pdf_break_level = 4


man_pages = [("index", "ns-3-model-library", "ns-3 Model Library", ["ns-3 project"], 1)]
