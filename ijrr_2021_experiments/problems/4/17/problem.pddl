(define (problem ijrr_17)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street4 street35 - street
		building2 building7 building22 building27 - building
	)
	(:init
		(atLocation survivor0 street35)
		(atLocation survivor1 building7)
		(onFire building2)
		(onFire building22)
		(underRubble street4)
		(underRubble building27)
		(needsRepair building2)
		(needsRepair building22)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street4))
		(not (needsRepair building2))
		(not (needsRepair building22))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
