(domain (define survivor)
	(:requirements :typing)
	(:types
		location movable enum_number- object
		inside_warehouse outside_warehouse - location
		hospital survivor_start potential_fire - outside_warehouse
		fire damaged_building - potential_fire
		survivor medicine crate water_container construction_kit - movable
		small_crate large_crate - crate
		small_water_container large_water_container - water_container
	)

	(:constants
		PICKUP - location
		MEDICINE_START SMALL_CRATE_PACKAGING LARGE_CRATE_PACKAGING WATER_CONTAINER_START - inside_warehouse
		ZERO ONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN - enum_number
		NULL_C - crate
	)

	; Boolean functions
	(:predicates
		(movable ?m - movable)
		(used ?m - medicine)
		(healed ?s - survivor)
		(used ?m - medicine)
		(mutex ?m - movable)
		(onFire ?f - potential_fire)
		(rubbleCleared ?b - damaged_building)
		(repaired ?b - damaged_building)
	)

	; Functions with a discrete number of return values
	(:functions
		(location ?m - movable) - location
		(contains ?c - crate) - enum_number
		(in ?m - medicine) - crate
		(uses ?wc - large_water_container) - enum_number
	)

	; Calculated by an external module (e.g. the motion planner)
	; Could also be set as a numeric function through precomputing the time and setting the function in the initial state
	(:semantic_attachments
		(motion_time ?from - location ?to - location)
	)

	; Adds a medicine to an empty small crate
	(:durative-action addMedicineToSmallCrate01
		:parameters (?m - medicine ?sc - small_crate)
		:duration (= ?duration (motion_time MEDICINE_START SMALL_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING))

						; The crate starts as empty
						(at start (= (contains ?sc) ZERO))
				   )
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The small crate now contains one medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) SMALL_CRATE_PACKAGING))
					(at end (= (contains ?sc) ONE))
				 )
	)

	; Adds a medicine to a small crate with one medicine already in it
	(:durative-action addMedicineToSmallCrate12
		:parameters (?m - medicine ?sc - small_crate)
		:duration (= ?duration (motion_time MEDICINE_START SMALL_CRATE_PACKAGING))
		:condition (and
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING))

						; The crate starts with one medicine
						(at start (= (contains ?sc) ONE))
				   )
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The small crate now contains two medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) SMALL_CRATE_PACKAGING))
					(at end (= (contains ?sc) TWO))
				 )
	)

	; Adds a medicine to a small crate with two medicine already in it
	(:durative-action addMedicineToSmallCrate23
		:parameters (?m - medicine ?sc - small_crate)
		:duration (= ?duration (motion_time MEDICINE_START SMALL_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING))

						; The crate starts with two medicine
						(at start (= (contains ?sc) TWO))
				   )
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The small crate now contains three medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) SMALL_CRATE_PACKAGING))
					(at end (= (contains ?sc) THREE))
				 )
	)

	; Adds a medicine to a small crate with three medicine already in it
	(:durative-action addMedicineToSmallCrate34
		:parameters (?m - medicine ?sc - small_crate)
		:duration (= ?duration (motion_time MEDICINE_START SMALL_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING))

						; The crate starts with three medicine
						(at start (= (contains ?sc) THREE))
				   )
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The small crate now contains four medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) SMALL_CRATE_PACKAGING))
					(at end (= (contains ?sc) FOUR))
				 )
	)

	; Adds a medicine to a small crate with four medicine already in it
	(:durative-action addMedicineToSmallCrate45
		:parameters (?m - medicine ?sc - small_crate)
		:duration (= ?duration (motion_time MEDICINE_START SMALL_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING))

						; The crate starts with four medicine
						(at start (= (contains ?sc) FOUR))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The small crate now contains five medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) SMALL_CRATE_PACKAGING))
					(at end (= (contains ?sc) FIVE))
				 )
	)

	; Adds a medicine to an empty large crate
	(:durative-action addMedicineToLargeCrate01
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with zero medicine
						(at start (= (contains ?lc) ZERO))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains one medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) ONE))
				 )
	)

	; Adds a medicine to a large crate with one medicine already in it
	(:durative-action addMedicineToLargeCrate12
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with one medicine
						(at start (= (contains ?lc) ONE))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains two medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) TWO))
				 )
	)

	; Adds a medicine to a large crate with two medicine already in it
	(:durative-action addMedicineToLargeCrate23
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with two medicine
						(at start (= (contains ?lc) TWO))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains three medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) THREE))
				 )
	)

	; Adds a medicine to a large crate with three medicine already in it
	(:durative-action addMedicineToLargeCrate34
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with three medicine
						(at start (= (contains ?lc) THREE))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains four medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) FOUR))
				 )
	)

	; Adds a medicine to a large crate with four medicine already in it
	(:durative-action addMedicineToLargeCrate45
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with four medicine
						(at start (= (contains ?lc) FOUR))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains five medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) FIVE))
				 )
	)

	; Adds a medicine to a large crate with five medicine already in it
	(:durative-action addMedicineToLargeCrate56
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with five medicine
						(at start (= (contains ?lc) FIVE))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains six medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) SIX))
				 )
	)

	; Adds a medicine to a large crate with six medicine already in it
	(:durative-action addMedicineToLargeCrate67
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with six medicine
						(at start (= (contains ?lc) SIX))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains seven medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) SEVEN))
				 )
	)

	; Adds a medicine to a large crate with seven medicine already in it
	(:durative-action addMedicineToLargeCrate78
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with seven medicine
						(at start (= (contains ?lc) SEVEN))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains eight medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) EIGHT))
				 )
	)

	; Adds a medicine to a large crate with eight medicine already in it
	(:durative-action addMedicineToLargeCrate89
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START LARGE_CRATE_PACKAGING))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with eight medicine
						(at start (= (contains ?lc) EIGHT))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains nine medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) NINE))
				 )
	)

	; Adds a medicine to a large crate with nine medicine already in it
	(:durative-action addMedicineToLargeCrate910
		:parameters (?m - medicine ?lc - large_crate)
		:duration (= ?duration (motion_time MEDICINE_START PICKUP))
		:condition (and 
						; The crate and the medicine need to be at their respective starting location
						(at start (= (location ?m) MEDICINE_START))
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING))

						; The crate starts with nine medicine
						(at start (= (contains ?lc) NINE))
					)
		:effect (and
					; Only one action can manipulate the crate or the medicine
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The large crate now contains ten medicine and
					; the medicine shares the same location as the crate
					(at end (= (location ?m) LARGE_CRATE_PACKAGING))
					(at end (= (contains ?lc) TEN)
				 )
	)

	; Moves a single medicine to the pickup location
	(:durative-action moveMedicineToPickup
		:parameters (?m - medicine)
		:duration (= ?duration motion_time(LARGE_CRATE_PACKAGING PICKUP))
		:condition (and
						; The medicine needs to be at its respective starting location
						(at start (= (location ?m) MEDICINE_START)
					)
		:effect (and
					; Only one action can manipulate the medicine
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The medicine is moved to the pickup location
					(at end (= (location ?m) PICKUP)
				)
	)

	; Moves a loaded small crate to the pickup location
	(:durative-action moveSmallCrateToPickup
		:parameters (?sc - small_crate ?m1 ?m2 ?m3 ?m4 ?m5 - medicine)
		:duration (= ?duration (motion_time SMALL_CRATE_PACKAGING PICKUP))
		:condition (and
						; The small crate must be loaded
						(at start (= (contains ?sc) FIVE))

						; The crate and its contents must be at the packaging location
						(at start (= (location ?sc) SMALL_CRATE_PACKAGING)
						(at start (= (location ?m1) SMALL_CRATE_PACKAGING)
						(at start (= (location ?m2) SMALL_CRATE_PACKAGING)
						(at start (= (location ?m3) SMALL_CRATE_PACKAGING)
						(at start (= (location ?m4) SMALL_CRATE_PACKAGING)
						(at start (= (location ?m5) SMALL_CRATE_PACKAGING)

						; Each of the medicine list must in this specific crate
						(at start (= (in ?m1) ?sc))
						(at start (= (in ?m2) ?sc))
						(at start (= (in ?m3) ?sc))
						(at start (= (in ?m4) ?sc))
						(at start (= (in ?m5) ?sc))
					)
		:effect (and
					; The crate and each of the medicine can only be manipulated by one action at a time
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m1))
					(at end (not (mutex ?m1)))
					(at start (mutex ?m2))
					(at end (not (mutex ?m2)))
					(at start (mutex ?m3))
					(at end (not (mutex ?m3)))
					(at start (mutex ?m4))
					(at end (not (mutex ?m4)))
					(at start (mutex ?m5))
					(at end (not (mutex ?m5)))

					; Move the crate and each of the medine to the pickup location
					(at end (= (location ?sc) PICKUP)
					(at end (= (location ?m1) PICKUP)
					(at end (= (location ?m2) PICKUP)
					(at end (= (location ?m3) PICKUP)
					(at end (= (location ?m4) PICKUP)
					(at end (= (location ?m5) PICKUP)
				)
	)

	; Moves a loaded large crate to the pickup location
	(:durative-action moveLargeCrateToPickup
		:parameters (?lc - large_crate ?m1 ?m2 ?m3 ?m4 ?m5 ?m6 ?m7 ?m8 ?m9 ?m10- medicine)
		:duration (= ?duration (motion_time LARGE_CRATE_PACKAGING PICKUP))
		:condition (and
						; The large crate must be loaded
						(at start (= (contains ?lc) TEN))

						; The crate and its contents must be at the packaging location
						(at start (= (location ?lc) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m1) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m2) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m3) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m4) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m5) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m6) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m7) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m8) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m9) LARGE_CRATE_PACKAGING)
						(at start (= (location ?m10) LARGE_CRATE_PACKAGING)

						; Each of the medicine must be in the crate
						(at start (= (in ?m1) ?sc))
						(at start (= (in ?m2) ?sc))
						(at start (= (in ?m3) ?sc))
						(at start (= (in ?m4) ?sc))
						(at start (= (in ?m5) ?sc))
						(at start (= (in ?m6) ?sc))
						(at start (= (in ?m7) ?sc))
						(at start (= (in ?m8) ?sc))
						(at start (= (in ?m9) ?sc))
						(at start (= (in ?m10) ?sc))
					)
		:effect (and
					; The crate and each of the medicine can only be manipulated by one action at a time
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m1))
					(at end (not (mutex ?m1)))
					(at start (mutex ?m2))
					(at end (not (mutex ?m2)))
					(at start (mutex ?m3))
					(at end (not (mutex ?m3)))
					(at start (mutex ?m4))
					(at end (not (mutex ?m4)))
					(at start (mutex ?m5))
					(at end (not (mutex ?m5)))
					(at start (mutex ?m6))
					(at end (not (mutex ?m6)))
					(at start (mutex ?m7))
					(at end (not (mutex ?m7)))
					(at start (mutex ?m8))
					(at end (not (mutex ?m8)))
					(at start (mutex ?m9))
					(at end (not (mutex ?m9)))
					(at start (mutex ?m10))
					(at end (not (mutex ?m10)))

					; The crate and its contents get moved to the pickup location
					(at end (= (location ?lc) PICKUP)
					(at end (= (location ?m1) PICKUP)
					(at end (= (location ?m2) PICKUP)
					(at end (= (location ?m3) PICKUP)
					(at end (= (location ?m4) PICKUP)
					(at end (= (location ?m5) PICKUP)
					(at end (= (location ?m6) PICKUP)
					(at end (= (location ?m7) PICKUP)
					(at end (= (location ?m8) PICKUP)
					(at end (= (location ?m9) PICKUP)
					(at end (= (location ?m10) PICKUP)
				)
	)

	; Transport a single medicine to a hospital
	(:durative-action moveMedicineToHospital
		:parameters (?m - medicine ?h - hospital)
		:duration (= ?duration (motion_time PICKUP ?h))
		:condition (and
						; The medicine is at the pickup location
						(at start (= (location ?m) PICKUP))

						; The medicine is not in any of the crates
						(at start (forall ?c - crate (not (= (in ?m) ?c))))
				   )
		:effect (and
					; The medicine can only be manipulated by one action at a time
			    	(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The medicine ends up at the hospital
					(at end (= (location ?m) ?h)
				)
	)

	; Transport a small crate to a hospital
	(:durative-action moveMedicineToHospital
		:parameters (?sc -small_crate ?m1 ?m2 ?m3 ?m4 ?m5 - medicine ?h - hospital)
		:duration (= ?duration motion_time(PICKUP ?h))
		:condition (and
						; The crate must be loaded
						(at start (= (contains ?sc) FIVE))

						; The crate and the medicine it contains is at the pickup location
						(at start (= (location ?m1) PICKUP))
						(at start (= (location ?m2) PICKUP))
						(at start (= (location ?m3) PICKUP))
						(at start (= (location ?m4) PICKUP))
						(at start (= (location ?m5) PICKUP))

						; The medicine is in the crates
						(at start (= (in ?m1) sc))
						(at start (= (in ?m2) sc))
						(at start (= (in ?m3) sc))
						(at start (= (in ?m4) sc))
						(at start (= (in ?m5) sc))
				   )
		:effect (and
					; The crate and each of the medicine can only be manipulated by one action at a time
					(at start (mutex ?sc))
					(at end (not (mutex ?sc)))
					(at start (mutex ?m1))
					(at end (not (mutex ?m1)))
					(at start (mutex ?m2))
					(at end (not (mutex ?m2)))
					(at start (mutex ?m3))
					(at end (not (mutex ?m3)))
					(at start (mutex ?m4))
					(at end (not (mutex ?m4)))
					(at start (mutex ?m5))
					(at end (not (mutex ?m5)))

					; Move the crate and each of the medine to the hospital
					(at end (= (location ?sc) ?h)
					(at end (= (location ?m1) ?h)
					(at end (= (location ?m2) ?h)
					(at end (= (location ?m3) ?h)
					(at end (= (location ?m4) ?h)
					(at end (= (location ?m5) ?h)
				)
	)

	; Moves a loaded large crate to a hospital
	(:durative-action moveLargeCrateToPickup
		:parameters (?lc - large_crate ?m1 ?m2 ?m3 ?m4 ?m5 ?m6 ?m7 ?m8 ?m9 ?m10- medicine ?h - hospital)
		:duration (= ?duration (motion_time PICKUP ?h))
		:condition (and
						; The large crate must be loaded
						(at start (= (contains ?lc) TEN))

						; The crate and its contents must be at the [ickup location
						(at start (= (location ?lc) PICKUP)
						(at start (= (location ?m1) PICKUP)
						(at start (= (location ?m2) PICKUP)
						(at start (= (location ?m3) PICKUP)
						(at start (= (location ?m4) PICKUP)
						(at start (= (location ?m5) PICKUP)
						(at start (= (location ?m6) PICKUP)
						(at start (= (location ?m7) PICKUP)
						(at start (= (location ?m8) PICKUP)
						(at start (= (location ?m9) PICKUP)
						(at start (= (location ?m10) PICKUP)

						; Each of the medicine must be in the crate
						(at start (= (in ?m1) ?sc))
						(at start (= (in ?m2) ?sc))
						(at start (= (in ?m3) ?sc))
						(at start (= (in ?m4) ?sc))
						(at start (= (in ?m5) ?sc))
						(at start (= (in ?m6) ?sc))
						(at start (= (in ?m7) ?sc))
						(at start (= (in ?m8) ?sc))
						(at start (= (in ?m9) ?sc))
						(at start (= (in ?m10) ?sc))
					)
		:effect (and
					; The crate and each of the medicine can only be manipulated by one action at a time
					(at start (mutex ?lc))
					(at end (not (mutex ?lc)))
					(at start (mutex ?m1))
					(at end (not (mutex ?m1)))
					(at start (mutex ?m2))
					(at end (not (mutex ?m2)))
					(at start (mutex ?m3))
					(at end (not (mutex ?m3)))
					(at start (mutex ?m4))
					(at end (not (mutex ?m4)))
					(at start (mutex ?m5))
					(at end (not (mutex ?m5)))
					(at start (mutex ?m6))
					(at end (not (mutex ?m6)))
					(at start (mutex ?m7))
					(at end (not (mutex ?m7)))
					(at start (mutex ?m8))
					(at end (not (mutex ?m8)))
					(at start (mutex ?m9))
					(at end (not (mutex ?m9)))
					(at start (mutex ?m10))
					(at end (not (mutex ?m10)))

					; The crate and its contents get moved to the hospital
					(at end (= (location ?lc) ?h)
					(at end (= (location ?m1) ?h)
					(at end (= (location ?m2) ?h)
					(at end (= (location ?m3) ?h)
					(at end (= (location ?m4) ?h)
					(at end (= (location ?m5) ?h)
					(at end (= (location ?m6) ?h)
					(at end (= (location ?m7) ?h)
					(at end (= (location ?m8) ?h)
					(at end (= (location ?m9) ?h)
					(at end (= (location ?m10) ?h)
				)
	)

	; Move the survivor to the hospital
	(:durative-action moveSurvivorToHospital
		:parameters (?s - survivor ?h - hospital)
		:duration (= ?duration (motion_time (location ?s) ?h))
		:condition (and
						; The survivor is not already at the hospital
						(at start (not (= (location ?s) ?h)))

						; The survivor is not already healed
						(at start (not (healed ?s)))
		           )
		:effect (and
					; The survivor can only be manipulated by one action at a time
					(at start (mutex ?s))
					(at end (not (mutex ?s)))

					; The survivor ends up at the hospital
					(at end (= (location ?s) ?h))
		        )
	)

	; Heals a survivor
	(:durative-action healSurvivor
		:parameters (?s - survivor ?h - hospital ?m - medicine)
		:duration (= ?duration 1) ; Constant number that will be changed
		:condition (and
						; Both the survivor and the medicine should be at the hospital
						(at start (= (location ?s) ?h))
						(at start (= (location ?m) ?h))

						; The survivor needs healing
						(at start (not (healed ?s)))

						; The medicine has not been used yet
						(at start (not (used ?m)))
		           )
		:effect (and
					; The survivor and medicine can only be manipulated by one action at a time
					(at start (mutex ?s))
					(at end (not (mutex ?s)))
					(at start (mutex ?m))
					(at end (not (mutex ?m)))

					; The survivor is healed
					(at end (healed ?s))

					; The medicine is used up
					(at end (used ?m))
				)	
	)

	; Moves a water container to the pickup location
	(:durative-action moveWaterContainerToPickup
		:parameters (?wc - water_container)
		:duration (= ?duration (motion_time WATER_CONTAINER_START PICKUP))
		:condition (and
						; The water container starts at its starting location
						(at start (= (location ?wc) WATER_CONTAINER_START))
				   )
		:effect (and
					; The water container can only be manipulated by one action at a time
					(at start (mutex ?wc))
					(at end (not (mutex ?wc)))

					; The water container ends up at the pickup location
					(at end (= (location ?wc) PICKUP))
				)
	)

	; Moves a small water container to a fire
	(:durative-action moveSmallWaterContainerToFire
		:parameters (?wc - small_water_container ?f - potential_fire)
		:duration (= ?duration (motion_time PICKUP ?b))
		:condition (and
						; The water container starts at the pickup location
						(at start (= (location ?wc) PICKUP))

						; Location must be on fire
						(at start (onFire ?f))
				   )
		:effect (and
					; The water container can only be manipulated by one action at a time
					(at start (mutex ?wc))
					(at end (not (mutex ?wc)))

					; Moves the small water container
					(at end (= (location ?wc) ?f))
				)
	)

	; Moves the large water container from any location outside the warehouse to a fire
	(:durative-action moveLargeWaterContainerToFire
		:parameters (?wc - large_water_container ?f - potential_fire ?from - outside_warehouse)
		:duration (= ?duration (motion_time PICKUP ?f))
		:condition (and
						; The water container is not empty (been used twice)
						(at start (not (= (uses ?wc) TWO))

						; The water container is at the from
						(at start (= (location ?wc) ?from))

						; The current location is not on fire
						(at start (not (onFire ?from)))
				   )
		:effect (and
					; The water container can only be manipulated by one action at a time
					(at start (mutex ?wc))
					(at end (not (mutex ?wc)))

					; Moves the water container
					(at end (= (location ?wc) ?f))
				)
	)

	; Puts out a fire
	(:durative-action putOutFire1
		:parameters (?wc - water_container ?f - potential_fire)
		:duration (= ?duration 1) ; constant duration that will be changed
		:condition (and
						; Water container needs to be at the fire location
						(at start (= (location ?wc) ?f))

						; The location needs to be on fire
						(at start (onFire ?f))

						; The water container needs to not have been used
						(at start (= (uses ?wc) ZERO))
				   )
		:effect (and
					; The water container can only be manipulated by one action at a time
					(at start (mutex ?wc))
					(at end (mutex ?wc))

					; Sets the number of uses for the water container to one
					(at end (= (uses ?wc) ONE))

					; Puts out the fire
					(at end (not (onFire ?f)))
				)
	)

	; The large water container can be used to put out a second fire
	(:durative-action putOutFire2
		:parameters (?wc - large_water_container ?f - potential_fire)
		:duration (= ?duration 1) ; constant duration that will be changed
		:condition (and
						; Water container needs to be at the fire location
						(at start (= (location ?wc) ?f))

						; The location needs to be on fire
						(at start (onFire ?f))

						; This action runs if the water container has been used once
						(at start (= (uses ?wc) ONE))
				   )
		:effect (and
					; The water container can only be manipulated by one action at a time
					(at start (mutex ?wc))
					(at end (mutex ?wc))

					; Sets the number of uses for the water container to one
					(at end (= (uses ?wc) TWO))

					; Puts out the fire
					(at end (not (onFire ?f)))
				)
	)

	; Clears the rubble from a damaged building
	(:durative-action clearRubble
		:parameters (?b - damaged_building)
		:duration (= ?duration 1) ; constant duration that will be changed
		:condition (and
						; The building cannot be on fire
						(at start (not (onFire ?b)))

						; There must be rubble
						(at start (not (rubbleCleared ?b)))
				   )
		:effect (and
					; Rubble is cleared
					(at end (rubbleCleared ?b))
				)
	)

	; Repairs a building
	(:durative-action repairBuilding
		:parameters (?b - damaged_building)
		:duration (= ?duration 1) ; constant duration that will be changed
		:condition (and
						; The building cannot be on fire
						(at start (not (onFire ?b)))

						; Rubble must have been cleared
						(at start (rubbleCleared ?b))

						; Building cannot have already been repaired
						(at start (not (repaired ?b)))
				   )
		:effect (and
					; Building is repaired
					(at end (repaired ?b))
				)
	)
)