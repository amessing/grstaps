(define (problem ijrr_17)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street24 street35 - street
		building11 building13 building17 building18 - building
	)
	(:init
		(atLocation survivor0 street24)
		(atLocation survivor1 building11)
		(onFire building13)
		(onFire building18)
		(underRubble street35)
		(underRubble building17)
		(needsRepair building13)
		(needsRepair building17)
		(needsRepair building18)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street35))
		(not (needsRepair building13))
		(not (needsRepair building17))
		(not (needsRepair building18))
	))
	(:metric minimize total-time)
)
