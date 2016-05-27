#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/enumeration.py
# Enumeration of support profiles for a strategic game
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

class SupportEnumeration(object):
    def enumerate_supports(self, game):
        return self.admissible_supports(game.support_profile(), list(game.strategies))

    def admissible_supports(self, profile, str_rest):
        # Passo 1: closure
        while True:
            temp_profile = profile.undominated(True, True)
            if temp_profile == profile:
                break
            profile = temp_profile

        # Step 2: y' and z'
        new_rest = filter(lambda x: x in list(profile), str_rest)

        # Step 3: return x if z' is empty
        if not new_rest:
            yield profile

        # Step 4: recursive step if z' isn't empty
        else:
            elem = new_rest.pop()

            for gen in self.admissible_supports(profile, new_rest):
                yield gen

            try:
                new_profile = profile.remove(elem)
            except ValueError:
                raise StopIteration

            for gen in self.admissible_supports(new_profile, new_rest):
                yield gen
