@routing @guidance @turn-lanes
Feature: Turn Lane Guidance

    Background:
        Given the profile "car"
        Given a grid size of 10 meters

    Scenario: Basic Turn Lane Turn
        Given the node map
            | a |   | b |   | c |
            |   |   | d |   |   |

        And the ways
            | nodes  | turn:lanes     | turn:lanes:forward | turn:lanes:backward | name     |
            | ab     | through\|right |                    |                     | in       |
            | bc     |                |                    | left\|through       | straight |
            | bd     |                |                    |                     | right    |

       When I route I should get
            | waypoints | route                | turns                           |
            | a,c       | in,straight,straight | depart,new name straight,arrive |
            | a,d       | in,right,right       | depart,turn right,arrive        |
