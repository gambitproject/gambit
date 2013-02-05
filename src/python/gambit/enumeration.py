#
# Enumerates support profiles of a strategic game
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
