@routing @guidance @turn-lanes
Feature: Turn Lane Guidance

    Background:
        Given the profile "car"
        Given a grid size of 10 meters

    #requires https://github.com/cucumber/cucumber-js/issues/417
    #Due to this, we use & as a pipe character. Switch them out for \| when 417 is fixed
    @bug @WORKAROUND-FIXME
    Scenario: Basic Turn Lane 3-way Turn with empty lanes
        Given the node map
            | a |   | b |   | c |
            |   |   | d |   |   |

        And the ways
            | nodes  | turn:lanes     | turn:lanes:forward | turn:lanes:backward | name     |
            | ab     |                | through\|right     |                     | in       |
            | bc     |                |                    | left\|through&&     | straight |
            | bd     |                |                    | left\|right         | right    |

       When I route I should get
            | waypoints | route                | turns                           |
            | a,c       | in,straight,straight | depart,new name straight,arrive |
            | a,d       | in,right,right       | depart,turn right,arrive        |

    Scenario: Basic Turn Lane 4-Way Turn
        Given the node map
            |   |   | e |   |   |
            | a |   | b |   | c |
            |   |   | d |   |   |

        And the ways
            | nodes  | turn:lanes     | turn:lanes:forward | turn:lanes:backward | name     |
            | ab     |                | \|right            |                     | in       |
            | bc     |                |                    |                     | straight |
            | bd     |                |                    | left\|              | right    |
            | be     |                |                    |                     | left     |

       When I route I should get
            | waypoints | route                | turns                           |
            | a,c       | in,straight,straight | depart,new name straight,arrive |
            | a,d       | in,right,right       | depart,turn right,arrive        |
            | a,e       | in,left,left         | depart,turn left,arrive         |

    Scenario: Basic Turn Lane 4-Way Turn using none
        Given the node map
            |   |   | e |   |   |
            | a |   | b |   | c |
            |   |   | d |   |   |

        And the ways
            | nodes  | turn:lanes     | turn:lanes:forward | turn:lanes:backward | name     |
            | ab     |                | none\|right        |                     | in       |
            | bc     |                |                    |                     | straight |
            | bd     |                |                    | left\|none          | right    |
            | be     |                |                    |                     | left     |

       When I route I should get
            | waypoints | route                | turns                           |
            | a,c       | in,straight,straight | depart,new name straight,arrive |
            | a,d       | in,right,right       | depart,turn right,arrive        |
            | a,e       | in,left,left         | depart,turn left,arrive         |

    Scenario: Basic Turn Lane Turn
        Given the node map
            |   |   | e |   |   |
            | a |   | b |   | c |
            |   |   | d |   |   |

        And the ways
            | nodes  | turn:lanes     | turn:lanes:forward | turn:lanes:backward | name     |
            | ab     |                | none\|right        |                     | in       |
            | bc     |                |                    |                     | straight |
            | bd     |                |                    | left\|none          | right    |
            | be     |                |                    |                     | left     |

       When I route I should get
            | waypoints | route                | turns                           |
            | a,c       | in,straight,straight | depart,new name straight,arrive |
            | a,d       | in,right,right       | depart,turn right,arrive        |
            | a,e       | in,left,left         | depart,turn left,arrive         |

    @TODO @WORKAROUND-FIXME
    Scenario: Turn with Bus-Lane
        Given the node map
            | a |   | b |   | c |
            |   |   |   |   |   |
            |   |   | d |   |   |

        And the ways
            | nodes | name | turn:lanes:forward | lanes:psv |
            | ab    | road | through\|right&    | 1         |
            | bc    | road |                    |           |
            | bd    | turn |                    |           |

        When I route I should get
            | waypoints | route          | turns                    |
            | a,d       | road,turn,turn | depart,turn right,arrive |

    #turn lanes are often drawn at the incoming road, even though the actual turn requires crossing the intersection first
    @TODO @WORKAROUND-FIXME
    Scenario: Turn Lanes at Segregated Road
        Given the node map
            |   |   | i |   | l |   |   |
            |   |   |   |   |   |   |   |
            | h |   | g |   | f |   | e |
            |   |   |   |   |   |   |   |
            | a |   | b |   | c |   | d |
            |   |   |   |   |   |   |   |
            |   |   | j |   | k |   |   |

        And the ways
            | nodes | name  | turn:lanes:forward  | oneway |
            | ab    | road  | left\|through&right | yes    |
            | bc    | road  | left\|through       | yes    |
            | cd    | road  |                     | yes    |
            | ef    | road  | \|through&right     | yes    |
            | fg    | road  | left\|through       | yes    |
            | gh    | road  |                     | yes    |
            | ig    | cross |                     | yes    |
            | gb    | cross | through\|left       | yes    |
            | bj    | cross |                     | yes    |
            | kc    | cross | left\|through;right | yes    |
            | cf    | cross | left\|through       | yes    |
            | fl    | cross |                     | yes    |

        When I route I should get
            | waypoints | route             | turns                        |
            | a,j       | road,cross,cross  | depart,turn right,arrive     |
            | a,d       | road,road         | depart,arrive                |
            | a,l       | road,cross,cross  | depart,turn left,arrive      |
            | a,h       | road,road,road    | depart,continue uturn,arrive |
            | k,d       | cross,road,road   | depart,turn right,arrive     |
            | k,l       | cross,cross       | depart,arrive                |
            | k,h       | cross,road,road   | depart,turn left,arrive      |
            | k,j       | cross,cross,cross | depart,continue uturn,arrive |
            | e,l       | road,cross,cross  | depart,turn right,arrive     |
            | e,h       | road,road         | depart,arrive                |
            | e,j       | road,cross,cross  | depart,turn left,arrive      |
            | e,d       | road,road,road    | depart,continue uturn,arrive |
            | i,h       | cross,road,road   | depart,turn right,arrive     |
            | i,j       | cross,cross       | depart,arrive                |
            | i,d       | cross,road,road   | depart,turn left,arrive      |
            | i,l       | cross,cross,cross | depart,continue uturn,arrive |


    #this can happen due to traffic lights / lanes not drawn up to the intersection itself
    @TODO
    Scenario: Turn Lanes Given earlier than actual turn
        Given the node map
            | a |   | b | c |   | d |
            |   |   |   |   |   |   |
            |   |   |   | e |   |   |

        And the ways
            | nodes | name | turn:lanes:forward |
            | ab    | road | \|right            |
            | bc    | road |                    |
            | cd    | road |                    |
            | ce    | turn |                    |

        When I route I should get
            | waypoints | route          | turns                    |
            | a,e       | road,turn,turn | depart,turn right,arrive |


