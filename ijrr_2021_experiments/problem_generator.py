import json
from pathlib import Path
from shapely.geometry import Polygon
import random
import shutil

class Map:
    def __init__(self, buildings: list, roads: list):
        self.buildings = buildings
        self.roads = roads

    @staticmethod
    def load(map_name: str) -> 'Map':
        with open(map_name) as f:
            map_data = json.load(f)

        buildings = []
        for shape in map_data['buildings']:
            l = []
            for point in shape:
                l.append([point['x'], point['y']])

            poly = Polygon(l)
            buildings.append(poly.centroid)

        roads = []
        for shape in map_data['roads']:
            l = []
            for point in shape:
                l.append([point['x'], point['y']])

            poly = Polygon(l)
            roads.append(poly.centroid)

        return Map(buildings=buildings, roads=roads)

class Coord:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class Location:
    def __init__(self, name:str, coord: Coord, is_building:bool, survivor:int=None, fire:bool=False, rubble:bool=False):
        self.name = name
        self.coord = coord
        self.is_building = is_building
        self.survivor = survivor
        self.fire = fire
        self.rubble = rubble

class SpecialtyBuilding(Location):
    def __init__(self, name:str, coord:Coord, original_name:str):
        Location.__init__(self, name, coord, True)
        self.original_name = original_name # buildingXXX

class LocationManager:
    def __init__(self):
        self.streets = []
        self.buildings = []
        self.hospital = None
        self.fire_station = None
        self.construction_company = None

class CustomJsonEncoder(json.JSONEncoder):
    def default(self, obj):
        if (isinstance(obj, Location) or
            isinstance(obj, Coord)):
            return obj.__dict__
        # Let the base class default method raise the TypeError
        return json.JSONEncoder.default(self, obj)

class ProblemGenerator:
    def __init__(self, config: dict):
        # copy so it can be saved to file later
        self.config = config

        # Robots
        self.num_ambulances = config.get('num_ambulances')
        self.num_fire_trucks_ground = config.get('num_fire_trucks_ground')
        self.num_fire_trucks_aerial = config.get('num_fire_trucks_aerial')
        self.num_cranes_ground = config.get('num_cranes_ground')
        self.num_cranes_aerial = config.get('num_cranes_aerial')

        # Goal items
        self.num_survivors = config.get('num_survivors')
        self.percent_survivors_on_street = config.get('percent_survivors_on_street')
        self.num_fires = config.get('num_fires')
        self.max_water = config.get('max_water')
        self.num_rubble = config.get('num_rubble')
        self.percent_rubble_on_street = config.get('percent_rubble_on_street')


    def generate(self, parent_folder: str, problem_name: str, map_name: str):
        # Create the folder for the problem
        Path(f'{parent_folder}/{problem_name}').mkdir(parents=True, exist_ok=True)

        # Generate locations
        lm = self.generate_locations(map_name)

        # Copy the domain
        shutil.copyfile('domain.pddl', f'{parent_folder}/{problem_name}/domain.pddl')

        # Write the pddl problem file
        self.write_pddl_problem_file(parent_folder, problem_name, lm)

        # Write the json config
        self.write_config(parent_folder, problem_name, lm)

        # Copy the map
        shutil.copyfile(map_name, f'{parent_folder}/{problem_name}/map.json')

    def generate_locations(self, map_name: str):

        m = Map.load(map_name)
        
        num_streets = len(m.roads)
        print(f'{num_streets} streets')
        num_buildings = len(m.buildings)
        print(f'{num_buildings} buildings')

        lm = LocationManager()

        for i, street in enumerate(m.roads):
            c = Coord(street.x, street.y)
            l = Location(f'street{i}', c, False)
            lm.streets.append(l)

        for i, building in enumerate(m.buildings):
            c = Coord(building.x, building.y)
            l = Location(f'building{i}', c, True)
            lm.buildings.append(l)

        # Set parameters
        ## Survivors
        num_survivors_on_street = int(self.num_survivors * self.percent_survivors_on_street)
        num_survivors_in_buildings = self.num_survivors - num_survivors_on_street
        ## Rubble
        num_rubble_on_street = int(self.num_rubble * self.percent_rubble_on_street)
        num_rubble_in_buildings = self.num_rubble - num_rubble_on_street

        # Add survivors to streets
        survivor_nr = 0
        for index in random.choices(range(num_streets), k=num_survivors_on_street):
            lm.streets[index].survivor = survivor_nr
            survivor_nr += 1

        # No fire on streets

        # Add rubble to streets
        for index in random.choices(range(num_streets), k=num_rubble_on_street):
            lm.streets[index].rubble = True

        # Create specialty buildings (hospital, fire station, construction company)
        ## Hospital
        index = random.randrange(num_buildings)
        original_building = lm.buildings[index]
        del lm.buildings[index]
        lm.hospital = SpecialtyBuilding('hospital', original_building.coord, original_building.name)
        ## Fire Station
        index = random.randrange(num_buildings - 1)
        original_building = lm.buildings[index]
        del lm.buildings[index]
        lm.fire_station = SpecialtyBuilding('fire_station', original_building.coord, original_building.name)
        ## Construction Company
        index = random.randrange(num_buildings - 2)
        original_building = lm.buildings[index]
        del lm.buildings[index]
        lm.construction_company = SpecialtyBuilding('construction_company', original_building.coord, original_building.name)

        # Add survivors to buildings
        for index in random.choices(range(num_buildings - 3), k=num_survivors_in_buildings):
            lm.buildings[index].survivor = survivor_nr
            survivor_nr += 1

        for index in random.choices(range(num_buildings - 3), k=self.num_fires):
            lm.buildings[index].fire = True

        for index in random.choices(range(num_buildings - 3), k=num_rubble_in_buildings):
            lm.buildings[index].rubble = True

        return lm

    def write_pddl_problem_file(self, parent_folder: str, problem_name: str, lm: LocationManager):
        with open(f'{parent_folder}/{problem_name}/problem.pddl', 'w') as f:
            # Write header
            f.write(f'(define (problem ijrr_{problem_name})\n\t(:domain ijrr)\n')

            # Write objects
            f.write('\t(:objects\n\t\t')

            ## Write survivors
            for i in range(self.num_survivors):
                f.write(f'survivor{i} ')
            f.write('- survivor\n\t\t')

            ## Write locations (only the important ones)
            for street in lm.streets:
                if street.survivor is not None or street.fire or street.rubble:
                    f.write(f'{street.name} ')
            f.write('- street\n\t\t')
            for building in lm.buildings:
                if building.survivor is not None or building.fire or building.rubble:
                    f.write(f'{building.name} ')
            f.write('- building\n')

            f.write('\t)\n')
            
            # Write init
            f.write('\t(:init\n')
            ## Survivor initial locations
            for street in lm.streets:
                if street.survivor is not None:
                    f.write(f'\t\t(atLocation survivor{street.survivor} {street.name})\n')
            for building in lm.buildings:
                if building.survivor is not None:
                    f.write(f'\t\t(atLocation survivor{building.survivor} {building.name})\n')
            ## What is on fire
            for building in lm.buildings:
                if building.fire:
                    f.write(f'\t\t(onFire {building.name})\n')
            ## What is under rubble
            for street in lm.streets:
                if street.rubble:
                    f.write(f'\t\t(underRubble {street.name})\n')
            for building in lm.buildings:
                if building.rubble:
                    f.write(f'\t\t(underRubble {building.name})\n')
            ## Needs repair (fire or rubble)
            for building in lm.buildings:
                if building.fire or building.rubble:
                    f.write(f'\t\t(needsRepair {building.name})\n')
            f.write('\t)\n')

            # Write Goal
            f.write('\t(:goal (and\n')
            ## Survivor -> Hospital
            for i in range(self.num_survivors):
                f.write(f'\t\t(atLocation survivor{i} HOSPITAL)\n')
            ## Rubble cleared from all roads
            for street in lm.streets:
                if street.rubble:
                    f.write(f'\t\t(not (underRubble {street.name}))\n')
            ## All buildings repaired
            for building in lm.buildings:
                if building.fire or building.rubble:
                    f.write(f'\t\t(not (needsRepair {building.name}))\n')
            f.write('\t))\n')


            f.write('\t(:metric minimize total-time)\n')
            f.write(')\n')

    def write_config(self, parent_folder: str, problem_name: str, lm: LocationManager):
        config = {
            # Locations
            'streets': [],
            'buildings': [],
            'hospital': lm.hospital,
            'fire_station': lm.fire_station,
            'construction_company': lm.construction_company,
            # Traits: MP, speed, payload, water capacity, construction ability
            'ambulance_traits':         [0, 3, 5, 0, 0],
            'fire_truck_ground_traits': [0, 3, 3, 3, 1.1],
            'fire_truck_aerial_traits': [1, 6, 1.1, 1.1, 1.1],
            'crane_ground_traits':      [0, 2, 10, 0, 10],
            'crane_aerial_traits':      [1, 4, 3.4, 2, 3.4],
            'mp_index': 0,
            'speed_index': 1,
            # Robots
            'robots': [],
            'robot_traits': [],
            'robot_start_locations': [],
            # Trait Requirements
            'moveSurvivor_trait_requirements':       [0, 0.1, 5, 0, 0],
            'exstinguishFire_trait_requirements':    [0, 0.1, 0, 1, 0],
            'exstinguishFire2_trait_requirements':   [0, 0.1, 0, 1, 0],
            'exstinguishFire3_trait_requirements':   [0, 0.1, 0, 1, 0],
            'clearRubble_trait_requirements':        [0, 0.1, 2, 0, 1],
            'clearRubble2_trait_requirements':       [0, 0.1, 4, 0, 1],
            'clearRubble3_trait_requirements':       [0, 0.1, 6, 0, 1],
            'repairBuilding_trait_requirements':     [0, 0.1, 0, 0, 9],
            'repairBuilding2_trait_requirements':    [0, 0.1, 0, 0, 18],
            'repairBuilding3_trait_requirements':    [0, 0.1, 0, 0, 27],
            # Grounded Actions
            'grounded_actions': [],
            'actions_trait_requirements': [],
            'actions_start_end': []
        }
        for street in lm.streets:
            #if street.survivor is not None or street.fire or street.rubble:
            config['streets'].append(street)
        for building in lm.buildings:
            #if building.survivor is not None or building.fire or building.rubble:
            config['buildings'].append(building)
        hospital_index = len(config['streets']) + len(config['buildings'])
        fire_station_index = hospital_index + 1
        construction_company_index = fire_station_index + 1

        # List traits
        for i in range(self.num_ambulances):
            config['robots'].append(f'ambulance{i}')
            config['robot_traits'].append(config['ambulance_traits'])
            config['robot_start_locations'].append(hospital_index)
        for i in range(self.num_fire_trucks_ground):
            config['robots'].append(f'fire_truck_ground{i}')
            config['robot_traits'].append(config['fire_truck_ground_traits'])
            config['robot_start_locations'].append(fire_station_index)
        for i in range(self.num_fire_trucks_aerial):
            config['robots'].append(f'fire_truck_aerial{i}')
            config['robot_traits'].append(config['fire_truck_aerial_traits'])
            config['robot_start_locations'].append(fire_station_index)
        for i in range(self.num_cranes_ground):
            config['robots'].append(f'crane_ground{i}')
            config['robot_traits'].append(config['crane_ground_traits'])
            config['robot_start_locations'].append(construction_company_index)
        for i in range(self.num_cranes_aerial):
            config['robots'].append(f'crane_aerial{i}')
            config['robot_traits'].append(config['crane_aerial_traits'])
            config['robot_start_locations'].append(construction_company_index)

        # List action trait requirements
        location_index = 0
        for street in lm.streets:
            if street.survivor is not None:
                # Move to hospital
                config['grounded_actions'].append(f'movesurvivortohospital survivor{street.survivor} {street.name}')
                config['actions_trait_requirements'].append(config['moveSurvivor_trait_requirements'])
                config['actions_start_end'].append([location_index, hospital_index])
            if street.rubble:
                # Clear rubble
                config['grounded_actions'].append(f'clearrubble {street.name}')
                config['actions_trait_requirements'].append(config['clearRubble_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                config['grounded_actions'].append(f'clearrubble2 {street.name}')
                config['actions_trait_requirements'].append(config['clearRubble2_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                config['grounded_actions'].append(f'clearrubble3 {street.name}')
                config['actions_trait_requirements'].append(config['clearRubble3_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])
            location_index += 1
        for building in lm.buildings:
            if building.survivor is not None:
                # Move to hospital
                config['grounded_actions'].append(f'movesurvivortohospital survivor{building.survivor} {building.name}')
                config['actions_trait_requirements'].append(config['moveSurvivor_trait_requirements'])
                config['actions_start_end'].append([location_index, hospital_index])
            if building.fire:
                # Put out fire
                water = random.randrange(self.max_water) + 1
                config['grounded_actions'].append(f'exstinguishfire {building.name}')
                config['actions_trait_requirements'].append(config['exstinguishFire_trait_requirements'])
                config['actions_trait_requirements'][-1][3] = water
                config['actions_start_end'].append([fire_station_index, location_index])

                # Put out fire 2
                config['grounded_actions'].append(f'exstinguishfire2 {building.name}')
                config['actions_trait_requirements'].append(config['exstinguishFire2_trait_requirements'])
                config['actions_trait_requirements'][-1][3] = water * 2
                config['actions_start_end'].append([fire_station_index, location_index])

                # Put out fire 3
                config['grounded_actions'].append(f'exstinguishfire3 {building.name}')
                config['actions_trait_requirements'].append(config['exstinguishFire3_trait_requirements'])
                config['actions_trait_requirements'][-1][3] = water * 3
                config['actions_start_end'].append([fire_station_index, location_index])
            if building.rubble:
                # Clear rubble
                config['grounded_actions'].append(f'clearrubble {building.name}')
                config['actions_trait_requirements'].append(config['clearRubble_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                # Clear rubble2
                config['grounded_actions'].append(f'clearrubble2 {building.name}')
                config['actions_trait_requirements'].append(config['clearRubble2_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                # Clear rubble3
                config['grounded_actions'].append(f'clearrubble3 {building.name}')
                config['actions_trait_requirements'].append(config['clearRubble3_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])
            if building.fire or building.rubble:
                # Repair building
                config['grounded_actions'].append(f'repairbuilding {building.name}')
                config['actions_trait_requirements'].append(config['repairBuilding_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                # Repair building 2
                config['grounded_actions'].append(f'repairbuilding2 {building.name}')
                config['actions_trait_requirements'].append(config['repairBuilding2_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])

                # Repair building 3
                config['grounded_actions'].append(f'repairbuilding3 {building.name}')
                config['actions_trait_requirements'].append(config['repairBuilding3_trait_requirements'])
                config['actions_start_end'].append([location_index, location_index])
            location_index += 1
        
        with open(f'{parent_folder}/{problem_name}/config.json', 'w') as f:
            json.dump(config, f, cls=CustomJsonEncoder, indent=4)

        # original parameters
        with open(f'{parent_folder}/{problem_name}/parameters.json', 'w') as f:
            json.dump(self.config, f, indent=4)
def main():
    for problem_nr in range(5):
        config = {
            'num_ambulances': problem_nr + 1,
            'num_fire_trucks_ground': problem_nr + 1,
            'num_fire_trucks_aerial': problem_nr + 1,
            'num_cranes_ground': problem_nr + 1,
            'num_cranes_aerial': problem_nr + 1,
            'num_survivors': 10,
            'percent_survivors_on_street': 0.5,
            'num_fires': 5,
            'max_water': 3,
            'num_rubble': 5,
            'percent_rubble_on_street': 0.5
        }

        pg = ProblemGenerator(config)
        for instance_nr in range(20):
            pg.generate(f'problems/{problem_nr}', f'{instance_nr}', 'maps/test.json')

if  __name__ == "__main__":
    main()