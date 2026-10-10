# -*- coding: utf-8 -*-

import sys, os


extensions = ["sphinx.ext.imgmath", "sphinxcontrib.seqdiag"]

templates_path = ["_templates"]

source_suffix = ".rst"


master_doc = "buildings"

project = "LENA"
copyright = "2011-2012, CTTC"

version = "M2"
release = "M2"


exclude_patterns = []


pygments_style = "sphinx"


html_theme = "default"


latex_documents = [
    (
        "buildings",
        "buildings.tex",
        "Buildings Module Documentation",
        "Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)",
        "manual",
    ),
]


man_pages = [("index", "ns-3-model-library", "ns-3 Model Library", ["ns-3 project"], 1)]
