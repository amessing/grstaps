(define (problem ijrr_14)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street34 - street
		building6 building8 building29 building31 - building
	)
	(:init
		(atLocation survivor0 street34)
		(atLocation survivor1 building29)
		(onFire building6)
		(onFire building8)
		(underRubble street0)
		(underRubble building31)
		(needsRepair building6)
		(needsRepair building8)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street0))
		(not (needsRepair building6))
		(not (needsRepair building8))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
