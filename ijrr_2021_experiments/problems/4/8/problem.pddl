(define (problem ijrr_8)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street43 - street
		building25 building33 building34 - building
	)
	(:init
		(atLocation survivor0 street0)
		(atLocation survivor1 building33)
		(onFire building25)
		(onFire building34)
		(underRubble street43)
		(underRubble building33)
		(needsRepair building25)
		(needsRepair building33)
		(needsRepair building34)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street43))
		(not (needsRepair building25))
		(not (needsRepair building33))
		(not (needsRepair building34))
	))
	(:metric minimize total-time)
)
