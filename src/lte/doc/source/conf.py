# -*- coding: utf-8 -*-

import sys, os


extensions = ["sphinx.ext.imgmath"]

templates_path = ["_templates"]

source_suffix = ".rst"


master_doc = "lte"

project = "LENA"
copyright = "CTTC"

version = "v10"
release = "v10"


exclude_patterns = []


pygments_style = "sphinx"


html_theme = "default"


latex_documents = [
    (
        "lte",
        "lena-lte-module-doc.tex",
        "The LENA ns-3 LTE Module Documentation",
        "Centre Tecnològic de Telecomunicacions de Catalunya (CTTC)",
        "manual",
    ),
]


pdf_break_level = 4


man_pages = [("index", "ns-3-model-library", "ns-3 Model Library", ["ns-3 project"], 1)]
