(define (problem ijrr_13)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street15 street52 - street
		building6 building15 building33 building36 - building
	)
	(:init
		(atLocation survivor0 street15)
		(atLocation survivor1 building6)
		(onFire building15)
		(onFire building33)
		(underRubble street52)
		(underRubble building36)
		(needsRepair building15)
		(needsRepair building33)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street52))
		(not (needsRepair building15))
		(not (needsRepair building33))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
