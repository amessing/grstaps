(define (problem ijrr_10)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street1 street47 - street
		building8 building9 building12 building33 - building
	)
	(:init
		(atLocation survivor0 street1)
		(atLocation survivor1 building33)
		(onFire building8)
		(onFire building12)
		(underRubble street47)
		(underRubble building9)
		(needsRepair building8)
		(needsRepair building9)
		(needsRepair building12)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street47))
		(not (needsRepair building8))
		(not (needsRepair building9))
		(not (needsRepair building12))
	))
	(:metric minimize total-time)
)
