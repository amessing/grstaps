(define (problem ijrr_1)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street39 street40 - street
		building24 building28 building31 building33 - building
	)
	(:init
		(atLocation survivor0 street39)
		(atLocation survivor1 building31)
		(onFire building24)
		(onFire building28)
		(underRubble street40)
		(underRubble building33)
		(needsRepair building24)
		(needsRepair building28)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street40))
		(not (needsRepair building24))
		(not (needsRepair building28))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
