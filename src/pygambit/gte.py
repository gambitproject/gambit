#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/gte.py
# File format interface with Game Theory Explorer
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

"""
File format interface with Game Theory Explorer
"""

from fractions import Fraction
try:
    from lxml import etree
    has_lxml = True
except ImportError:
    has_lxml = False
    
import pygambit.lib.libgambit

def read_game_subtree(game, node, xml_node):
    if xml_node.get("player") is None:
        node.append_move(game.players.chance, len(xml_node))
    elif xml_node.get("iset") is not None:
        p = game.players[int(xml_node.get("player"))-1]
        try:
            node.append_move(p.infosets[xml_node.get("iset")])
        except IndexError:
            iset = node.append_move(p, len(xml_node))
            iset.label = xml_node.get("iset")
    elif xml_node.get("player") is not None:
        node.append_move(game.players[int(xml_node.get("player"))-1],
                         len(xml_node))
    for (i, xml_child) in enumerate(xml_node):
        if xml_child.get("prob") is not None:
            node.infoset.actions[i].prob = Fraction(xml_child.get("prob"))
        if xml_child.get("move") is not None:
            node.infoset.actions[i].label = xml_child.get("move")
        if xml_child.tag == "outcome":
            node.children[i].outcome = game.outcomes.add()
            for (j, xml_payoff) in enumerate(xml_child.xpath("./payoff")):
                node.children[i].outcome[int(xml_payoff.get("player"))-1] = Fraction(xml_payoff.text)
        elif xml_child.tag == "node":
            read_game_subtree(game, node.children[i], xml_child)

def read_game(f):
    if not has_lxml:
        raise NotImplementedError("Reading and writing GTE files requires lxml module.")
    tree = etree.parse(f)
    if tree.xpath("/gte/@version")[0] != "0.1":
        raise ValueError("GTE reader only supports version 0.1")

    g = pygambit.lib.libgambit.new_tree()
    for p in tree.xpath("/gte/players/player"):
        g.players.add(p.text)

    read_game_subtree(g, g.root, tree.xpath("/gte/extensiveForm/node")[0])
    return g

def write_game_outcome(game, outcome, doc, xml_parent):
    for (i, p) in enumerate(game.players):
        if outcome is not None:
            etree.SubElement(xml_parent, "payoff",
                             player=p.label).text = str(outcome[i])
        else:
            etree.SubElement(xml_parent, "payoff",
                             player=p.label).text = "0"

def write_game_node(game, node, doc, xml_node):
    if len(node.infoset.members) >= 2:
        xml_node.set("iset", node.infoset.label)
    if not node.infoset.is_chance:
        xml_node.set("player", node.player.label)
    for (i, child) in enumerate(node.children):
        if child.is_terminal:
            xml_child = etree.SubElement(xml_node, "outcome")
            write_game_outcome(game, child.outcome, doc, xml_child)
        else:
            xml_child = etree.SubElement(xml_node, "node")
            write_game_node(game, child, doc, xml_child)
        if node.infoset.is_chance:
            xml_child.set("prob", str(node.infoset.actions[i].prob))
        xml_child.set("move", node.infoset.actions[i].label)

def write_game_display(game, doc, xml_display):
    for (i, p) in enumerate(game.players):
        color = etree.SubElement(xml_display, "color",
                                 player=str(i+1))
        if i % 2 == 0:
            color.text = "#FF0000"
        else:
            color.text = "#0000FF"
    etree.SubElement(xml_display, "font").text = "Times"
    etree.SubElement(xml_display, "strokeWidth").text = "1"
    etree.SubElement(xml_display, "nodeDiameter").text = "7"
    etree.SubElement(xml_display, "isetDiameter").text = "25"
    etree.SubElement(xml_display, "levelDistance").text = "75"
        
def write_game(game):
    if not has_lxml:
        raise NotImplementedError("Reading and writing GTE files requires lxml module.")
    gte = etree.Element("gte", version="0.1")
    doc = etree.ElementTree(gte)
    etree.SubElement(gte, "gameDescription")
    display = etree.SubElement(gte, "display")
    write_game_display(game, doc, display)
    players = etree.SubElement(gte, "players")
    for (i, p) in enumerate(game.players):
        etree.SubElement(players, "player",
                         playerId=str(i+1)).text = p.label
    efg = etree.SubElement(gte, "extensiveForm")
    xml_root = etree.SubElement(efg, "node")
    write_game_node(game, game.root, doc, xml_root)
                         
    return etree.tostring(doc, pretty_print=True)


    
