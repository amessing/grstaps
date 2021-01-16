(define (problem ijrr_19)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street14 street40 - street
		building10 building13 building24 building33 - building
	)
	(:init
		(atLocation survivor0 street14)
		(atLocation survivor1 building33)
		(onFire building13)
		(onFire building24)
		(underRubble street40)
		(underRubble building10)
		(needsRepair building10)
		(needsRepair building13)
		(needsRepair building24)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street40))
		(not (needsRepair building10))
		(not (needsRepair building13))
		(not (needsRepair building24))
	))
	(:metric minimize total-time)
)
