(define (problem ijrr_7)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street32 street40 - street
		building0 building2 building3 building9 - building
	)
	(:init
		(atLocation survivor0 street32)
		(atLocation survivor1 building0)
		(onFire building2)
		(onFire building9)
		(underRubble street40)
		(underRubble building3)
		(needsRepair building2)
		(needsRepair building3)
		(needsRepair building9)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street40))
		(not (needsRepair building2))
		(not (needsRepair building3))
		(not (needsRepair building9))
	))
	(:metric minimize total-time)
)
