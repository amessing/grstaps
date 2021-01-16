(define (problem ijrr_18)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street44 - street
		building7 building12 building22 building29 - building
	)
	(:init
		(atLocation survivor0 street44)
		(atLocation survivor1 building29)
		(onFire building7)
		(onFire building22)
		(underRubble street2)
		(underRubble building12)
		(needsRepair building7)
		(needsRepair building12)
		(needsRepair building22)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street2))
		(not (needsRepair building7))
		(not (needsRepair building12))
		(not (needsRepair building22))
	))
	(:metric minimize total-time)
)
