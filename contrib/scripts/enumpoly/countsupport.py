
import random
import enumphc
import randomnfg

if __name__ == '__main__':
    import sys, os

    #os.system("rm game-*.nfg")

    argv = sys.argv
    solve = True

    if argv[1] == '-e':
        solve = False
        argv = [ argv[0] ] + argv[2:]
    
    game = randomnfg.CreateNfg([int(x) for x in argv[2:]])
    strategies = [ strategy
                   for player in game.Players()
                   for strategy in player.Strategies() ]

    print "ownuser,ownsystem,childuser,childsystem,supports,singular,nash,nonnash"
    for iter in xrange(int(argv[1])):
        randomnfg.RandomizeGame(game, lambda: random.normalvariate(0, 1))
        #file("game-%04d.nfg" % iter, "w").write(game.AsNfgFile())
        logger = enumphc.CountLogger()
        startTime = os.times()
        enumphc.EnumViaPHC(game, "blek", logger, solve=solve)
        endTime = os.times()

        print ("%f,%f,%f,%f,%d,%d,%d,%d" %
               (endTime[0] - startTime[0],
                endTime[1] - startTime[1],
                endTime[2] - startTime[2],
                endTime[3] - startTime[3],
                logger.candidates,
                logger.singulars,
                logger.nash,
                logger.nonnash))
        sys.stdout.flush()

