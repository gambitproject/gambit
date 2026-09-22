"""Shared pybtex citation style, registered as the "keystyle" plugin.

Imported by both this project's ``conf.py`` and ``catalog/doc/conf.py`` so the two
independent Sphinx projects render ``:cite:p:`` citations identically without
duplicating the style definition.
"""
import re

import pybtex.plugin
from pybtex.richtext import Symbol, Text
from pybtex.style.formatting import toplevel
from pybtex.style.formatting.alpha import Style as AlphaStyle
from pybtex.style.labels import BaseLabelStyle
from pybtex.style.template import (
    field,
    first_of,
    join,
    optional,
    optional_field,
    sentence,
    tag,
    words,
)


def dashify(text):
    dash_re = re.compile(r"-+")
    return Text(Symbol("ndash")).join(text.split(dash_re))


pages = field("pages", apply_func=dashify)


class KeyLabelStyle(BaseLabelStyle):
    """Custom label style that uses the BibTeX entry key as the citation label."""

    def format_labels(self, sorted_entries):
        for entry in sorted_entries:
            yield entry.key


class CustomAlphaStyle(AlphaStyle):
    """Custom formatting style that uses KeyLabelStyle for labels."""

    default_label_style = "keystyle"
    default_name_style = "lastfirst"

    def format_author_or_editor(self, e, as_sentence=False):
        return first_of[
            optional[self.format_names("author", as_sentence=as_sentence)],
            optional[self.format_editor(e, as_sentence=as_sentence)],
        ]

    def format_editor(self, e, as_sentence=True):
        editors = self.format_names("editor", as_sentence=False)
        if "editor" not in e.persons:
            return editors
        word = "(eds)" if len(e.persons["editor"]) > 1 else "(ed.)"
        result = join(sep=" ")[editors, word]
        if as_sentence:
            return sentence[result]
        return result

    def get_article_template(self, e):
        return toplevel[
            sentence[
                join(sep=", ")[
                    join(sep=" ")[
                        self.format_names("author", as_sentence=False),
                        field("year"),
                    ],
                    join["‘", field("title"), "’"],
                    tag("em")[field("journal")],
                    optional[words["vol.", field("volume")]],
                    optional[words["no.", field("number")]],
                    optional[words["pp.", pages]],
                ]
            ],
            sentence[optional_field("note")],
            self.format_web_refs(e),
        ]

    def get_book_template(self, e):
        return toplevel[
            sentence[
                join(sep=", ")[
                    join(sep=" ")[
                        self.format_author_or_editor(e, as_sentence=False),
                        field("year"),
                    ],
                    tag("em")[field("title")],
                    optional[words[field("edition"), "edn"]],
                    field("publisher"),
                    optional_field("address"),
                ]
            ],
            sentence[optional_field("note")],
            self.format_web_refs(e),
        ]

    def get_incollection_template(self, e):
        return toplevel[
            sentence[
                join(sep=", ")[
                    join(sep=" ")[
                        self.format_names("author", as_sentence=False),
                        field("year"),
                    ],
                    join["‘", field("title"), "’"],
                    words[
                        "in",
                        join(sep=", ")[
                            optional[self.format_editor(e, as_sentence=False)],
                            tag("em")[field("booktitle")],
                            optional[words["vol.", field("volume")]],
                        ],
                    ],
                    field("publisher"),
                    optional_field("address"),
                    optional[words["pp.", pages]],
                ]
            ],
            sentence[optional_field("note")],
            self.format_web_refs(e),
        ]

    def get_inbook_template(self, e):
        return self.get_incollection_template(e)

    def get_inproceedings_template(self, e):
        return toplevel[
            sentence[
                join(sep=", ")[
                    join(sep=" ")[
                        self.format_names("author", as_sentence=False),
                        field("year"),
                    ],
                    join["‘", field("title"), "’"],
                    tag("em")[field("booktitle")],
                    optional[words["pp.", pages]],
                ]
            ],
            sentence[optional_field("note")],
            self.format_web_refs(e),
        ]

    def get_techreport_template(self, e):
        type_and_number = optional[
            words[
                first_of[optional_field("type"), "Technical Report"],
                field("number"),
            ]
        ]
        return toplevel[
            sentence[
                join(sep=", ")[
                    join(sep=" ")[
                        self.format_names("author", as_sentence=False),
                        field("year"),
                    ],
                    join["‘", field("title"), "’"],
                    type_and_number,
                    field("institution"),
                    optional_field("address"),
                ]
            ],
            sentence[optional_field("note")],
            self.format_web_refs(e),
        ]


pybtex.plugin.register_plugin("pybtex.style.labels", "keystyle", KeyLabelStyle)
pybtex.plugin.register_plugin("pybtex.style.formatting", "keystyle", CustomAlphaStyle)
