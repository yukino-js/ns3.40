# -*- coding: utf-8 -*-

import sys, os


extensions = ["sphinx.ext.imgmath"]

templates_path = ["_templates"]

source_suffix = ".rst"


master_doc = "index"

project = "LENA"
copyright = "2011-2012, CTTC"

version = "ns-3-dev"
release = "ns-3-dev"


exclude_patterns = []


pygments_style = "sphinx"


html_theme = "default"


latex_documents = [
    (
        "antenna",
        "antenna.tex",
        "Antenna Module Documentation",
        "Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)",
        "manual",
    ),
]


man_pages = [("index", "ns-3-model-library", "ns-3 Model Library", ["ns-3 project"], 1)]
