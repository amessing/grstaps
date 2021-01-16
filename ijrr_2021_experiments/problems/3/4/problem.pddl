(define (problem ijrr_4)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street55 - street
		building6 building10 building15 building28 - building
	)
	(:init
		(atLocation survivor0 street55)
		(atLocation survivor1 building28)
		(onFire building6)
		(onFire building10)
		(underRubble street2)
		(underRubble building15)
		(needsRepair building6)
		(needsRepair building10)
		(needsRepair building15)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street2))
		(not (needsRepair building6))
		(not (needsRepair building10))
		(not (needsRepair building15))
	))
	(:metric minimize total-time)
)
